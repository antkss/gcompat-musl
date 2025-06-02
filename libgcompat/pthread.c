#define _GNU_SOURCE
#include <errno.h>   /* errno */
#include <fcntl.h>   /* O_CLOEXEC, O_RDONLY */
#include <pthread.h> /* pthread_atfork */
#include <sched.h>   /* sched_yield, CPU_ALLOC, CPU_FREE */
#include <unistd.h>  /* open, read */

#include "alias.h" /* weak_alias */

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
static inline void normalize_timespec(struct timespec *ts) {
    while (ts->tv_nsec >= 1000000000L) {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec++;
    }
    while (ts->tv_nsec < 0) {
        ts->tv_nsec += 1000000000L;
        ts->tv_sec--;
    }
}


// Forward declarations if these were in a separate header
// int pthread_clockjoin_np(pthread_t thread, void **retval, clockid_t clockid, const struct timespec *abstime);
// int pthread_cond_clockwait(pthread_cond_t *cond, pthread_mutex_t *mutex, clockid_t clockid, const struct timespec *abstime);

/**
 * @brief Helper function to compare two timespec structures.
 * @return -1 if ts1 < ts2, 0 if ts1 == ts2, 1 if ts1 > ts2.
 */
static inline int timespec_compare(const struct timespec *ts1, const struct timespec *ts2) {
    if (ts1->tv_sec < ts2->tv_sec) return -1;
    if (ts1->tv_sec > ts2->tv_sec) return 1;
    if (ts1->tv_nsec < ts2->tv_nsec) return -1;
    if (ts1->tv_nsec > ts2->tv_nsec) return 1;
    return 0;
}

/**
 * @brief Waits for a thread to terminate, with a timeout relative to a specific clock.
 *
 * This function is similar to glibc's pthread_clockjoin_np.
 * If abstime is NULL, it behaves like pthread_join.
 *
 * @param thread The thread to wait for.
 * @param retval If non-NULL, the exit status of the target thread is stored here.
 * @param clockid The clock ID to use for the timeout.
 * @param abstime The absolute time by which the thread should have terminated.
 * If NULL, waits indefinitely (like pthread_join).
 * @return 0 on success, ETIMEDOUT if the timeout expired, or another error number.
 */
int pthread_clockjoin_np(pthread_t thread, void **retval,
                         clockid_t clockid, const struct timespec *abstime) {
    // If abstime is NULL, behave like pthread_join.
    if (abstime == NULL) {
        return pthread_join(thread, retval); // pthread_join is a cancellation point.
    }

    // Validate abstime->tv_nsec if abstime is not NULL.
    if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000) {
        return EINVAL;
    }

    struct timespec current_time_custom_clock;
    // Define a short sleep interval for polling.
    // Shorter interval means more responsive to timeout, but more CPU usage.
    const struct timespec poll_interval = {.tv_sec = 0, .tv_nsec = 50 * 1000 * 1000}; // 50ms

    for (;;) {
        // Act as a cancellation point at the beginning of each poll cycle.
        pthread_testcancel();

        int join_ret = pthread_tryjoin_np(thread, retval);
        if (join_ret == 0) { // Thread terminated successfully.
            return 0;
        }
        if (join_ret != EBUSY) { // An error other than "still running".
            return join_ret;    // e.g., ESRCH (no such thread), EINVAL.
        }

        // Thread is still running (EBUSY). Check for timeout.
        if (clock_gettime(clockid, &current_time_custom_clock) == -1) {
            return errno; // Return error from clock_gettime.
        }

        // Compare current time with the absolute timeout.
        if (timespec_compare(&current_time_custom_clock, abstime) >= 0) {
            // Timeout has expired. Attempt one last non-blocking join.
            join_ret = pthread_tryjoin_np(thread, retval);
            if (join_ret == 0) return 0; // Terminated just in time.
            if (join_ret == EBUSY) return ETIMEDOUT; // Still busy, timeout confirmed.
            return join_ret; // Other error on last try.
        }

        // Timeout not yet expired. Calculate remaining time and sleep.
        long long remaining_ns;
        remaining_ns = (abstime->tv_sec - current_time_custom_clock.tv_sec) * 1000000000LL;
        remaining_ns += (abstime->tv_nsec - current_time_custom_clock.tv_nsec);

        if (remaining_ns <= 0) { // Should be caught by the check above, but as a safeguard.
             // Try one last time before declaring timeout.
            join_ret = pthread_tryjoin_np(thread, retval);
            if (join_ret == 0) return 0;
            if (join_ret == EBUSY) return ETIMEDOUT;
            return join_ret;
        }

        struct timespec actual_sleep_ts;
        long long poll_interval_ns = (long long)poll_interval.tv_sec * 1000000000LL + poll_interval.tv_nsec;

        if (remaining_ns < poll_interval_ns) {
            actual_sleep_ts.tv_sec = remaining_ns / 1000000000LL;
            actual_sleep_ts.tv_nsec = remaining_ns % 1000000000LL;
        } else {
            actual_sleep_ts = poll_interval;
        }
        
        // clock_nanosleep is a cancellation point.
        // Use CLOCK_MONOTONIC for reliable relative sleep.
        int sleep_ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &actual_sleep_ts, NULL);
        if (sleep_ret != 0 && sleep_ret != EINTR) {
            // An unexpected error occurred during sleep.
            return sleep_ret;
        }
        // If sleep was interrupted (EINTR) or completed, the loop continues to re-check.
    }
}

/**
 * @brief Waits on a condition variable, with a timeout relative to a specific clock.
 *
 * This function is similar to glibc's pthread_cond_clockwait.
 *
 * @param cond The condition variable to wait on.
 * @param mutex The mutex associated with the condition variable. Must be locked by the caller.
 * @param clockid The clock ID to use for the timeout.
 * @param abstime The absolute time by which the condition should have been signaled.
 * @return 0 if the condition was signaled, ETIMEDOUT if the timeout expired, or another error number.
 */
int pthread_cond_clockwait(pthread_cond_t *restrict cond,
                           pthread_mutex_t *restrict mutex,
                           clockid_t clockid,
                           const struct timespec *restrict abstime) {

    // Validate abstime->tv_nsec.
    if (abstime == NULL || abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000) {
        return EINVAL;
    }

    // If clockid is CLOCK_REALTIME, we can use pthread_cond_timedwait directly.
    if (clockid == CLOCK_REALTIME) {
        return pthread_cond_timedwait(cond, mutex, abstime);
    }

    struct timespec current_time_custom_clock;
    struct timespec current_time_realtime;
    struct timespec timeout_realtime; // Target absolute timeout for pthread_cond_timedwait

    for (;;) {
        // Act as a cancellation point.
        pthread_testcancel();

        // Get current time on the custom clock.
        if (clock_gettime(clockid, &current_time_custom_clock) == -1) {
            return errno; // Error from clock_gettime.
        }

        // Check if timeout has already expired on the custom clock.
        if (timespec_compare(&current_time_custom_clock, abstime) >= 0) {
            return ETIMEDOUT;
        }

        // Calculate remaining time (delta) on the custom clock.
        long long delta_ns;
        delta_ns = (abstime->tv_sec - current_time_custom_clock.tv_sec) * 1000000000LL;
        delta_ns += (abstime->tv_nsec - current_time_custom_clock.tv_nsec);

        // If delta_ns is zero or negative, timeout should have been caught above.
        // This is a safeguard.
        if (delta_ns <= 0) {
            return ETIMEDOUT;
        }

        // Get current time on CLOCK_REALTIME.
        if (clock_gettime(CLOCK_REALTIME, &current_time_realtime) == -1) {
            return errno; // Error from clock_gettime.
        }

        // Calculate the equivalent absolute timeout for CLOCK_REALTIME.
        // timeout_realtime = current_time_realtime + delta_ns.
        timeout_realtime.tv_sec = current_time_realtime.tv_sec + delta_ns / 1000000000LL;
        timeout_realtime.tv_nsec = current_time_realtime.tv_nsec + delta_ns % 1000000000LL;

        // Normalize timeout_realtime.tv_nsec.
        if (timeout_realtime.tv_nsec >= 1000000000) {
            timeout_realtime.tv_sec++;
            timeout_realtime.tv_nsec -= 1000000000;
        } else if (timeout_realtime.tv_nsec < 0) { // Should not happen if delta_ns > 0
            timeout_realtime.tv_sec--;
            timeout_realtime.tv_nsec += 1000000000;
        }
        
        // Ensure the calculated realtime timeout's nsec field is valid before passing to pthread_cond_timedwait
        if (timeout_realtime.tv_nsec < 0 || timeout_realtime.tv_nsec >= 1000000000) {
            // This indicates a potential issue with time calculation or extreme deltas.
            // Or if current_time_realtime + delta_ns results in a negative tv_sec
            // for timeout_realtime, which pthread_cond_timedwait might reject.
            // For very large delta_ns, tv_sec could overflow.
            // For simplicity, if this rare case occurs, we might have to return an error
            // or make the timeout slightly different.
            // Given delta_ns > 0, timeout_realtime should be in the future.
            // If timeout_realtime.tv_sec becomes negative due to huge delta from past,
            // pthread_cond_timedwait will likely return EINVAL.
            // Let's assume standard positive time values.
             return EINVAL; // Or a more specific error if calculation leads to invalid timespec
        }


        // pthread_cond_timedwait is a cancellation point.
        // It atomically unlocks the mutex, waits, and re-locks the mutex.
        int wait_ret = pthread_cond_timedwait(cond, mutex, &timeout_realtime);

        if (wait_ret == 0) { // Condition was signaled.
            return 0;
        }
        if (wait_ret == ETIMEDOUT) {
            // Timeout occurred according to CLOCK_REALTIME.
            // We must re-check against the custom clock's abstime, as CLOCK_REALTIME
            // might have jumped or drifted, or our calculated timeout_realtime might
            // have been slightly off. The loop will perform this re-check.
            // If the custom clock timeout is genuinely met, the check at the loop start will catch it.
            continue; // Loop to re-evaluate custom clock time.
        }
        // Another error occurred during pthread_cond_timedwait.
        return wait_ret;
    }
}
