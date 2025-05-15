#define _GNU_SOURCE
#include <errno.h>   /* errno */
#include <fcntl.h>   /* O_CLOEXEC, O_RDONLY */
#include <pthread.h> /* pthread_atfork */
#include <sched.h>   /* sched_yield, CPU_ALLOC, CPU_FREE */
#include <unistd.h>  /* open, read */

#include "alias.h" /* weak_alias */

/**
 * Underlying function for pthread_cleanup_push.
 */
void __pthread_register_cancel(void *buf)
{
}

/**
 * Underlying function for pthread_cleanup_push.
 */
void __pthread_unregister_cancel(void *buf)
{
}

/**
 * Register fork handlers.
 *
 * LSB 5.0: LSB-Core-generic/baselib---register-atfork.html
 */
int __register_atfork(void (*prepare)(void), void (*parent)(void),
                      void (*child)(void), void *__dso_handle)
{
	return pthread_atfork(prepare, parent, child);
}
weak_alias(__register_atfork, register_atfork);

/**
 * Get the name of a thread.
 */
// int pthread_getname_np(pthread_t thread, char *name, size_t len)
// {
// 	int fd = open("/proc/thread-self/comm", O_RDONLY | O_CLOEXEC);
// 	ssize_t n;
//
// 	if (fd < 0)
// 		return errno;
// 	n = read(fd, name, len);
// 	if (n < 0)
// 		return errno;
// 	/* If the trailing newline was not read, the buffer was too small. */
// 	if (n == 0 || name[n - 1] != '\n')
// 		return ERANGE;
// 	name[n - 1] = '\0';
//
// 	return 0;
// }

/**
 * Yield this thread.
 */
int pthread_yield(void)
{
	return sched_yield();
}

/**
 * Allocate a large enough CPU set
 */
cpu_set_t *__sched_cpualloc(size_t __count)
{
	return CPU_ALLOC(__count);
}

/**
 * Free a CPU set allocated by __sched_cpualloc
 */
void __sched_cpufree(cpu_set_t *__set)
{
	return CPU_FREE(__set);
}

/**
 * Gets the mutex kind (non-portable variant).
 */
int pthread_mutexattr_getkind_np(const pthread_mutexattr_t *attr, int *kind)
{
	return pthread_mutexattr_gettype(attr, kind);
}

/**
 * Sets the mutex kind (non-portable variant).
 */
int pthread_mutexattr_setkind_np(pthread_mutexattr_t *attr, int kind)
{
	if (kind > PTHREAD_MUTEX_ERRORCHECK || kind < PTHREAD_MUTEX_NORMAL)
		return EINVAL;

	return pthread_mutexattr_settype(attr, kind);
}
int pthread_cond_clockwait(pthread_cond_t *cond,
                           pthread_mutex_t *mutex,
                           clockid_t clock_id,
                           const struct timespec *abstime) {
    if (clock_id == CLOCK_REALTIME) {
        // If using CLOCK_REALTIME, just pass through
        return pthread_cond_timedwait(cond, mutex, abstime);
    } else if (clock_id == CLOCK_MONOTONIC) {
        // Convert CLOCK_MONOTONIC abstime to equivalent CLOCK_REALTIME abstime

        struct timespec now_mono, now_real;
        if (clock_gettime(CLOCK_MONOTONIC, &now_mono) != 0)
            return errno;
        if (clock_gettime(CLOCK_REALTIME, &now_real) != 0)
            return errno;

        // Compute the difference between abstime and now_mono
        time_t d_sec  = abstime->tv_sec  - now_mono.tv_sec;
        long   d_nsec = abstime->tv_nsec - now_mono.tv_nsec;

        if (d_nsec < 0) {
            d_sec--;
            d_nsec += 1000000000;
        }

        if (d_sec < 0 || (d_sec == 0 && d_nsec <= 0)) {
            return ETIMEDOUT;
        }

        // Add the delta to now_real to get CLOCK_REALTIME abstime
        struct timespec real_abstime;
        real_abstime.tv_sec  = now_real.tv_sec + d_sec;
        real_abstime.tv_nsec = now_real.tv_nsec + d_nsec;
        if (real_abstime.tv_nsec >= 1000000000) {
            real_abstime.tv_sec++;
            real_abstime.tv_nsec -= 1000000000;
        }

        return pthread_cond_timedwait(cond, mutex, &real_abstime);
    } else {
        return ENOTSUP;  // Other clocks not supported
    }
}
