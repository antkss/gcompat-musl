#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <limits.h> // For INT_MAX

// Assuming "pthread_impl.h" provides the following:
// - pthread_testcancel()
// - Atomic operations: a_dec(), a_inc(), a_cas(), a_spin()
// - __timedwait_cp(volatile int *addr, int val, clockid_t clk, const struct timespec *abs_time, int private_futex)
// These are internal to musl's threading implementation.
#include "pthread_impl.h"

static void cleanup_clockwait(void *p)
{
	// Decrement the waiter count associated with the semaphore.
	// 'p' points to sem->__val[1]
	a_dec(p);
}

int sem_clockwait(sem_t *restrict sem, clockid_t clockid, const struct timespec *restrict abs_timeout)
{
	pthread_testcancel(); // Check for cancellation before proceeding.

	// Attempt to acquire the semaphore without blocking.
	if (!sem_trywait(sem)) {
		return 0; // Successfully acquired.
	}

	// Spin for a short duration if the semaphore value (count) is zero
	// and there are no other waiters. This is an optimization for
	// very short waits, avoiding the overhead of a system call.
	// SEM_VALUE_MAX in musl is typically INT_MAX.
	// The check `!(sem->__val[0] & INT_MAX)` means "if count is 0" (ignoring potential MSB flags).
	int spins = 100;
	while (spins-- && !(sem->__val[0] & INT_MAX) && !sem->__val[1]) {
		a_spin(); // Architecture-specific spin-wait hint.
	}

	// Loop to wait for the semaphore if trywait initially failed or spin-wait ended.
	while (sem_trywait(sem)) {
		int r;
		// priv likely indicates if the semaphore is process-shared.
		// This value is taken from sem->__val[2] as used in the original sem_timedwait.
		int priv = sem->__val[2];

		// Increment the count of waiters for this semaphore.
		// sem->__val[1] is used for this purpose.
		a_inc(sem->__val + 1);

		// If the semaphore count (sem->__val[0]) is 0, atomically set it to 0x80000000.
		// This value (0x80000000) is what __timedwait_cp will wait for if sem->__val[0]
		// currently holds this value. It's part of musl's futex-based waiting mechanism.
		a_cas(sem->__val, 0, 0x80000000);

		pthread_cleanup_push(cleanup_clockwait, (void *)(sem->__val + 1));
		// Call the internal timed wait function.
		// This is the main change from sem_timedwait: passing the user-specified 'clockid'.
		r = __timedwait_cp(sem->__val, 0x80000000, clockid, abs_timeout, priv);
		pthread_cleanup_pop(1); // Execute cleanup handler: decrements waiter count.

		if (r) { // If __timedwait_cp returned an error (e.g., ETIMEDOUT, EINTR)
			errno = r;
			return -1;
		}
		// If __timedwait_cp returned 0, we were woken.
		// The loop will call sem_trywait() again to attempt to acquire the semaphore.
	}

	return 0; // Semaphore successfully acquired by sem_trywait in the loop.
}
