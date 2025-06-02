/*
 * -------------------------------------------------------------------
 * Copyright © 2005-2020 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * -------------------------------------------------------------------
 */

/* This is an adaptation of musl's random() implementation, adding
 * random_data buffers to make reentrant variants
 */

#include <stddef.h>	/* NULL, size_t */
#include <stdint.h>	/* int32_t, uint32_t */
#include <errno.h>	/* errno */
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* This code uses the same lagged fibonacci generator as the
 * original bsd random implementation except for the seeding
 * which was broken in the original
 */

/* This must use the same struct layout as used by glibc
 * and specified in LSB
 * ref: https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/libc-ddefs.html
 */
struct random_data {
	int32_t *x;		/* int32_t *fptr */
	int32_t *unused_1;	/* int32_t *rptr */
	int32_t *unused_2;	/* int32_t *state */
	int unused_3;		/* int rand_type */
	int unused_4;		/* int rand_deg */
	int unused_5;		/* int rand_sep */
	int32_t *unused_6;	/* int32_t *end_ptr */
};

static uint32_t lcg31(uint32_t x) {
	return (1103515245*x + 12345) & 0x7fffffff;
}

static uint64_t lcg64(uint64_t x) {
	return 6364136223846793005ull*x + 1;
}

int srandom_r(unsigned seed, struct random_data *buf) {
	int k;
	uint64_t s = seed;
	int n, i, j;

	if (buf == NULL) {
		return -1;
	}

	n = buf->x[-1]>>16;
	i = (buf->x[-1]>>8)&0xff;
	j = buf->x[-1]&0xff;

	if (n > 63) {
		return -1;
	} else if (n == 0) {
		buf->x[0] = s;
		return 0;
	}
	i = n == 31 || n == 7 ? 3 : 1;
	j = 0;
	for (k = 0; k < n; k++) {
		s = lcg64(s);
		buf->x[k] = s>>32;
	}
	/* make sure x contains at least one odd number */
	buf->x[0] |= 1;

	buf->x[-1] = (n<<16)|(i<<8)|j;

	return 0;
}

int initstate_r(unsigned seed, char *restrict state, size_t size,
		struct random_data *restrict buf) {
	int n;

	if (size < 8) {
		errno = EINVAL;
		return -1;
	}

	if (size < 32) {
		n = 0;
	} else if (size < 64) {
		n = 7;
	} else if (size < 128) {
		n = 15;
	} else if (size < 256) {
		n = 31;
	} else {
		n = 63;
	}
	buf->x = (int32_t*)state + 1;
	buf->x[-1] = (n<<16)|(3<<8); /* (n<<16)|(i<<8)|j */
	srandom_r(seed, buf);
	return 0;
}

int setstate_r(char *restrict state, struct random_data *restrict buf) {
	if (!state || !buf) {
		errno = EINVAL;
		return -1;
	}

	buf->x = (int32_t*)state + 1;
	return 0;
}

int random_r(struct random_data *restrict buf, int32_t *restrict result) {
	long k;
	int n, i, j;

	if (result == NULL || buf == NULL) {
		errno = EINVAL;
		return -1;
	}

	n = buf->x[-1]>>16;
	i = (buf->x[-1]>>8)&0xff;
	j = buf->x[-1]&0xff;

	if (n == 0) {
		k = buf->x[0] = lcg31(buf->x[0]);
		goto end;
	}
	buf->x[i] += buf->x[j];
	k = buf->x[i]>>1;
	if (++i == n) {
		i = 0;
	} if (++j == n) {
		j = 0;
	}
	buf->x[-1] = (n<<16)|(i<<8)|j;
end:
	*result = k;
	return 0;
}
int srand_called = 0;
void arc4random_buf(void *_buf, size_t nbytes) {
    unsigned char *buf = (unsigned char *)_buf;
    size_t i;

    // Seed rand() if it hasn't been seeded yet.
    // This is a basic seeding approach and may not be sufficient for all uses.
    // For truly unpredictable sequences even with rand(), seeding should be
    // done carefully, possibly with higher entropy sources if available
    // (though if those are available, getentropy() would be better).
    if (!srand_called) {
        srand((unsigned int)time(NULL)); // Seed with current time
        srand_called = 1;
    }

    // Fill the buffer with bytes from rand()
    for (i = 0; i < nbytes; i++) {
        // rand() typically returns a value between 0 and RAND_MAX.
        // RAND_MAX is at least 32767.
        // We can take bytes from the random number.
        // To make it slightly better than just taking the lowest byte,
        // we can call rand() multiple times if needed, though the quality
        // is still limited by rand() itself.
        if (i % sizeof(int) == 0) {
            // Get a new random integer every sizeof(int) bytes or when starting
            // This is a naive way to try and use more bits from rand()
            // but the fundamental weaknesses of rand() remain.
            int random_value = rand();
            for (size_t j = 0; j < sizeof(int) && (i + j) < nbytes; ++j) {
                buf[i + j] = ((unsigned char *)&random_value)[j];
            }
            i += (sizeof(int) - 1); // Advance i, outer loop will increment once more
        } else {
            // This case should ideally not be hit if the above logic is correct,
            // but as a fallback, fill with a single byte from rand().
            buf[i] = (unsigned char)(rand() & 0xFF);
        }
    }
}

/*
 * WARNING: This implementation of arc4random uses the rand()-based arc4random_buf
 * and is NOT CRYPTOGRAPHICALLY SECURE.
 */
uint32_t arc4random(void) {
    uint32_t val;
    // Ensure srand has been called by arc4random_buf if not already
    if (!srand_called) {
        arc4random_buf(&val, sizeof(val)); // This will call srand
    } else {
        arc4random_buf(&val, sizeof(val));
    }
    return val;
}
