#define _GNU_SOURCE /* mempcpy */
#include <assert.h> /* assert */
#include <dlfcn.h>  /* dlsym, RTLD_NEXT */
#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* SIZE_MAX */
#include <stdlib.h> /* rand_r */
#define strerror_r no_strerror_r
#include <string.h> /* memcpy, strcpy, strncat, strndup */
#undef strerror_r
#include <time.h>   /* time */
#include <unistd.h> /* getpid */

#include "alias.h" /* weak_alias */

/**
 * Copy bytes in memory, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---memcpy-chk-1.html
 */
void *__memcpy_chk(void *dest, const void *src, size_t n, size_t destlen)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(destlen >= n);
	if (dest < src) {
		assert((char *) dest + n <= (char *) src);
	} else {
		assert((char *) src + n <= (char *) dest);
	}

	return memcpy(dest, src, n);
}

/**
 * Copy bytes in memory with overlapping areas, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---memmove-chk-1.html
 */
void *__memmove_chk(void *dest, const void *src, size_t n, size_t destlen)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(destlen >= n);

	return memmove(dest, src, n);
}

/**
 * Copy bytes in memory.
 *
 * LSB 5.0: LSB-Core-generic/baselib---mempcpy.html
 */
void *__mempcpy(void *dest, const void *src, size_t n)
{
	return mempcpy(dest, src, n);
}

/**
 * Copy bytes in memory, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---mempcpy-chk-1.html
 */
void *__mempcpy_chk(void *dest, const void *src, size_t n, size_t destlen)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(destlen >= n);
	if (dest < src) {
		assert((char *) dest + n <= (char *) src);
	} else {
		assert((char *) src + n <= (char *) dest);
	}

	return mempcpy(dest, src, n);
}

/**
 * Set bytes in memory, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---memset-chk-1.html
 */
void *__memset_chk(void *s, int c, size_t n, size_t buflen)
{
	assert(s != NULL);
	assert(buflen >= n);

	return memset(s, c, n);
}

void __explicit_bzero_chk(void *s, size_t n, size_t buflen)
{
	assert(s != NULL);
	assert(buflen >= n);

	explicit_bzero(s, n);
}

/**
 * Find byte in memory.
 *
 * LSB 5.0: LSB-Core-generic/baselib---rawmemchr.html
 */
void *__rawmemchr(const void *s, int c)
{
  if ((unsigned char) c != '\0')
    return memchr (s, c, (size_t)-1);
  return (char *)s + strlen (s);
}
weak_alias(__rawmemchr, rawmemchr);

/**
 * Copy a string and return a pointer to the end of the result, with buffer
 * overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---stpcpy-chk-1.html
 */
char *__stpcpy_chk(char *dest, const char *src, size_t destlen)
{
	size_t n;

	assert(dest != NULL);
	assert(src != NULL);
	n = strlen(src) + 1;
	assert(destlen >= n);
	if (dest < src) {
		assert(dest + n <= src);
	} else {
		assert(src + n <= dest);
	}

	return stpcpy(dest, src);
}

/**
 * Copy a fixed-length string, returning a pointer to the array end, with buffer
 * overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---stpncpy-chk-1.html
 */
char *__stpncpy_chk(char *dest, const char *src, size_t n, size_t destlen)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(destlen >= n);
	if (dest < src) {
		assert(dest + n <= src);
	} else {
		assert(src + n <= dest);
	}

	return stpncpy(dest, src, n);
}

/**
 * Concatenate two strings, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---strcat-chk-1.html
 */
char *__strcat_chk(char *dest, const char *src, size_t destlen)
{
	size_t n;
	size_t total;

	assert(dest != NULL);
	assert(src != NULL);
	n = strlen(src) + 1;
	total = strnlen(dest, destlen) + n;
	assert(destlen >= total);
	if (dest < src) {
		assert(dest + total <= src);
	} else {
		assert(src + n <= dest);
	}

	return strcat(dest, src);
}

/**
 * Copy a string, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---strcpy-chk-1.html
 */
char *__strcpy_chk(char *dest, const char *src, size_t destlen)
{
	size_t n;

	assert(dest != NULL);
	assert(src != NULL);
	n = strlen(src) + 1;
	assert(destlen >= n);
	if (dest < src) {
		assert(dest + n <= src);
	} else {
		assert(src + n <= dest);
	}

	return strcpy(dest, src);
}

/**
 * Find the substring length of a string that does not have any two characters.
 *
 * Not defined in LSB 5.0.  Used by spotify-client.
 */
size_t __strcspn_c2(const char *str, int bad, int bad2)
{
	size_t length = 0;
	const char *s = str;
	while (*s != bad && *s != bad2 && *s != '\0') {
		length++;
		s++;
	}
	return length;
}

/**
 * Alias for strdup.
 *
 * LSB 5.0: LSB-Core-generic/baselib---strdup-1.html
 */
char *__strdup(const char *string)
{
	return strdup(string);
}

/**
 * Non-POSIX strerror_r.
 */
// static int (*real_strerror_r)(int, char *, size_t);
//
// char *strerror_r(int errnum, char *buf, size_t buflen)
// {
// 	if (real_strerror_r == NULL) {
// 		real_strerror_r = dlsym(RTLD_NEXT, "strerror_r");
// 		assert(real_strerror_r);
// 	}
// 	real_strerror_r(errnum, buf, buflen);
// 	return buf;
// }

/**
 * Concatenate a string with part of another, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---strncat-chk-1.html
 */
char *__strncat_chk(char *dest, const char *src, size_t n, size_t destlen)
{
	size_t total;

	assert(dest != NULL);
	assert(src != NULL);
	total = strnlen(dest, destlen) + strnlen(src, destlen) + 1;
	assert(destlen >= total);
	if (dest < src) {
		assert(dest + total <= src);
	} else {
		assert(src + n <= dest);
	}

	return strncat(dest, src, n);
}

/**
 * Copy a fixed-length string, with buffer overflow checking.
 *
 * LSB 5.0: LSB-Core-generic/baselib---strncpy-chk-1.html
 */
char *__strncpy_chk(char *dest, const char *src, size_t n, size_t destlen)
{
	assert(dest != NULL);
	assert(src != NULL);
	assert(destlen >= n);
	if (dest < src) {
		assert(dest + n <= src);
	} else {
		assert(src + n <= dest);
	}

	return strncpy(dest, src, n);
}

/**
 * Duplicate a specific number of bytes from a string.
 */
char *__strndup(const char *s, size_t size)
{
	return strndup(s, size);
}

/**
 * Extract token from string.
 *
 * The "global" definition of strsep in glibc, used when architecture dependent
 * assembler versions aren't good enough.
 */
char *__strsep_g(char **stringp, const char *delim)
{
	return strsep(stringp, delim);
}

/**
 * Split string into tokens.
 *
 * LSB 5.0: LSB-Core-generic/baselib---strtok-r-1.html
 */
char *__strtok_r(char *s, const char *delim, char **save_ptr)
{
	return strtok_r(s, delim, save_ptr);
}

void *memfrob(void *s, size_t n)
{
	unsigned char *c = s;

	while (n--)
		*c++ ^= 42;

	return s;
}

char *strfry(char *s)
{
	static unsigned int seed;
	size_t len = strlen(s);

	if (!len)
		return s;

	seed += time(NULL) ^ getpid() ^ (uintptr_t) s;

	for (size_t i = 0; i < len - 1; ++i) {
		size_t j = rand_r(&seed) % (len - i) + i;
		char t = s[i];
		s[i] = s[j];
		s[j] = t;
	}

	return s;
}
