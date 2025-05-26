#include <features.h>
#define _GNU_SOURCE
#include <ctype.h>
#include <inttypes.h>
#include <strings.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#define EBADF 9
size_t __mbstowcs_chk(wchar_t *dst, const char *src, size_t len, size_t dstlen) {
    size_t result;

    if (dst == NULL || src == NULL)
        return (size_t)-1;

    result = mbstowcs(dst, src, dstlen);
    
    if (result > len) {
        // Buffer overflow detected
        abort();  // mimic __chk_fail
    }

    return result;
}
size_t __mbsrtowcs_chk(wchar_t *dst, const char **src, size_t len, mbstate_t *ps, size_t dstlen) {
    size_t result;

    if (dst == NULL || src == NULL || *src == NULL)
        return (size_t)-1;

    result = mbsrtowcs(dst, src, dstlen, ps);

    if (result > len) {
        abort();  // overflow
    }

    return result;
}
long __isoc23_strtol(const char *nptr, char **endptr, int base) {
    return strtol(nptr, endptr, base);  // delegate to standard strtol
}
size_t __wcsrtombs_chk(char *dst, const wchar_t **src, size_t len, mbstate_t *ps, size_t dstlen) {
    size_t result;

    if (dst == NULL || src == NULL || *src == NULL)
        return (size_t)-1;

    result = wcsrtombs(dst, src, dstlen, ps);

    if (result > len) {
        abort();  // buffer overflow
    }

    return result;
}
uintmax_t __isoc23_strtoumax(const char *nptr, char **endptr, int base) {
    return strtoumax(nptr, endptr, base);
}


void *pvalloc(size_t size) {
    size_t page_size = getpagesize();
    size_t rounded_size = (size + page_size - 1) & ~(page_size - 1);
    return malloc(rounded_size);
}

// hidden int __dn_expand(const unsigned char *msg, const unsigned char *eom, const unsigned char *src, char *dst, int dstsiz) {
//     // Simplified version, real implementation would require working with compressed DNS names
//     if (strlen((char *)src) > dstsiz) return -1;
//     strcpy(dst, (char *)src);
//     return strlen(dst);
// }
//
extern const char * const sys_errlist[];

const char * const sys_errlist[] = {
    "lmaodark",
    "Operation not lmao",
	"no such dark or lmao",
	"lmao dark dark",
};

const int sys_nerr = sizeof(sys_errlist) / sizeof(sys_errlist[0]);
int __res_query(const char *domain, int class, int type, unsigned char *answer, int anslen) {
    return res_query(domain, class, type, answer, anslen);
}

int parse_printf_format(const char *format, va_list args) {
    // A very simplified version that assumes correct format
    return vprintf(format, args);
}

int __ppoll_chk(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask, size_t sigset_size) {
    if (sigset_size < sizeof(sigset_t)) return -1;
    return ppoll(fds, nfds, timeout, sigmask);
}
int __res_nsearch(const char *domain, int class, int type, unsigned char *answer, int anslen) {
    return res_search(domain, class, type, answer, anslen);  // Correct function usage
}
size_t __libc_alloca_cutoff = 16384;
void *__libc_alloca_cutoff_func(size_t size) {
    if (size > __libc_alloca_cutoff) return NULL;
    return alloca(size);
}
intmax_t __isoc23_strtoimax(const char *nptr, char **endptr, int base) {
    return strtoimax(nptr, endptr, base);
}

// char *__stpncpy(char *dst, const char *src, size_t n) {
//     char *d = dst;
//     while (n-- && (*d++ = *src++));
//     return d;  // Returns a pointer to the character after the copied string
// }
unsigned long __isoc23_strtoul(const char *str, char **endptr, int base) {
	return strtol(str, endptr, base);
}
int errno = 0;


int
__libc_ns_makecanon (const char *src, char *dst, size_t dstsize)
{
  size_t n = strlen (src);

  if (n + sizeof "." > dstsize) /* sizeof == 2.  */
    {
      return -1;
    }
  strcpy (dst, src);
  while (n >= 1U && dst[n - 1] == '.')   /* Ends in ".".  */
    if (n >= 2U && dst[n - 2] == '\\' && /* Ends in "\.".  */
        (n < 3U || dst[n - 3] != '\\'))  /* But not "\\.".  */
      break;
    else
      dst[--n] = '\0';
  dst[n++] = '.';
  dst[n] = '\0';
  return 0;
}
int
__libc_ns_samename (const char *a, const char *b)
{
  char ta[NS_MAXDNAME], tb[NS_MAXDNAME];

  if (__libc_ns_makecanon (a, ta, sizeof ta) < 0 ||
      __libc_ns_makecanon (b, tb, sizeof tb) < 0)
    return -1;
  if (strcasecmp(ta, tb) == 0)
    return 1;
  else
    return 0;
}

#define SYS_gettid 186  // Linux syscall number for gettid

int pthread_attr_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset) {
    // Get thread ID (TID) using syscall for musl libc
    pid_t tid = syscall(SYS_gettid);  // syscall(SYS_gettid) gets the thread ID

    if (tid == -1) {
        return errno;  // Return error if thread ID retrieval fails
    }

    // Set the CPU affinity for the thread
    if (sched_setaffinity(tid, cpusetsize, cpuset) != 0) {
        return errno;  // Return error if setting affinity fails
    }

    return 0;  // Success
}
void __assert(const char *file, int line, const char *func, const char *failedexpr) {
    fprintf(stderr, "Assertion failed: %s, function %s, file %s, line %d.\n", failedexpr, func, file, line);
    abort();  // Terminate the program
}

int __isoc23_fscanf(FILE *stream, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int result = vfscanf(stream, format, ap);
    va_end(ap);
    return result;
}

long long __isoc23_strtoll(const char *nptr, char **endptr, int base) {
    return strtoll(nptr, endptr, base);
}

int __isoc23_vfscanf(FILE *stream, const char *format, va_list arg) {
    return vfscanf(stream, format, arg);
}

unsigned long __isoc23_wcstoul(const wchar_t *nptr, wchar_t **endptr, int base) {
    return wcstoul(nptr, endptr, base);
}

long long __isoc23_strtoll_l(const char *nptr, char **endptr, int base, locale_t loc) {
    (void)loc;
    return strtoll(nptr, endptr, base);
}

int __isoc23_vscanf(const char *format, va_list arg) {
    return vscanf(format, arg);
}

int __isoc23_scanf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int result = vscanf(format, ap);
    va_end(ap);
    return result;
}

unsigned long long __isoc23_strtoull_l(const char *nptr, char **endptr, int base, locale_t loc) {
    (void)loc;
    return strtoull(nptr, endptr, base);
}

long long __isoc23_wcstoll(const wchar_t *nptr, wchar_t **endptr, int base) {
    return wcstoll(nptr, endptr, base);
}

unsigned long long __isoc23_wcstoull(const wchar_t *nptr, wchar_t **endptr, int base) {
    return wcstoull(nptr, endptr, base);
}

int __isoc23_vsscanf(const char *s, const char *format, va_list arg) {
    return vsscanf(s, format, arg);
}

int __isoc23_sscanf(const char *s, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int result = vsscanf(s, format, ap);
    va_end(ap);
    return result;
}

unsigned long __isoc23_strtoul_l(const char *nptr, char **endptr, int base, locale_t loc) {
    (void)loc;
    return strtoul(nptr, endptr, base);
}

long __isoc23_wcstol(const wchar_t *nptr, wchar_t **endptr, int base) {
    return wcstol(nptr, endptr, base);
}

unsigned long long __isoc23_strtoull(const char *nptr, char **endptr, int base) {
    return strtoull(nptr, endptr, base);
}
int close_range(unsigned int fd_start, unsigned int fd_end, int flags) {
    int ret = 0;
    for (unsigned int fd = fd_start; fd <= fd_end; ++fd) {
        if (close(fd) == -1 && errno != EBADF)
            ret = -1;  // Record at least one failure
    }
    return ret;
}
int renameat2(int olddirfd, const char *oldpath,
              int newdirfd, const char *newpath,
              unsigned int flags)
{
    return syscall(SYS_renameat2, olddirfd, oldpath, newdirfd, newpath, flags);
}
int __openat64_2(int dirfd, const char *pathname, int flags, ...) {
    va_list ap;
    mode_t mode = 0;
    int fd;

    va_start(ap, flags);
    // If O_CREAT or O_TMPFILE is set, mode_t argument must be passed
    if ((flags & O_CREAT) || (flags & O_TMPFILE)) {
        mode = va_arg(ap, mode_t);
        fd = syscall(SYS_openat, dirfd, pathname, flags, mode);
    } else {
        fd = syscall(SYS_openat, dirfd, pathname, flags);
    }
    va_end(ap);

    return fd;
}
ssize_t __readlinkat_chk(int dirfd, const char *pathname, char *buf, size_t bufsiz, size_t bufsize_real) {
    if (bufsiz > bufsize_real) {
        // buffer overflow risk, abort or error
        __builtin_trap();
    }
    return readlinkat(dirfd, pathname, buf, bufsiz);
}
int pidfd_open(pid_t pid, unsigned int flags) {
    return syscall(SYS_pidfd_open, pid, flags);
}
int pidfd_send_signal(int pidfd, int sig, siginfo_t *info, unsigned int flags) {
    return syscall(SYS_pidfd_send_signal, pidfd, sig, info, flags);
}
int epoll_pwait2(int epfd, struct epoll_event *events, int maxevents,
                 const struct timespec *timeout, const sigset_t *sigmask) {
    return syscall(SYS_epoll_pwait2, epfd, events, maxevents, timeout, sigmask);
}





