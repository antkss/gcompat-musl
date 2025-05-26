#include "pthread_impl.h"
#include <threads.h>
#include <unistd.h>
extern int mincore (void *addr, size_t len, unsigned char *vec);
int is_mapped(void *addr) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("sysconf_SC_PAGESIZE");
        return -1;
    }
    void *page_aligned_addr = (void *)((uintptr_t)addr & ~(page_size - 1));
    unsigned char vec[1]; 
    return mincore(page_aligned_addr, page_size, vec);
}

static int __pthread_detach(pthread_t t)
{
	if (is_mapped(t) != 0) return 0;
	/* If the cas fails, detach state is either already-detached
	 * or exiting/exited, and pthread_join will trap or cleanup. */
	if (a_cas(&t->detach_state, DT_JOINABLE, DT_DETACHED) != DT_JOINABLE) {
		int cs;
		__pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
		__pthread_join(t, 0);
		__pthread_setcancelstate(cs, 0);
	}
	return 0;
}

weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);
