#include <errno.h> /* errno, ENOSYS */


struct __ucontext;

extern int  getcontext(struct __ucontext *);
extern void makecontext(struct __ucontext *, void (*)(), int, ...);
extern int  setcontext(const struct __ucontext *);
extern int  swapcontext(struct __ucontext *, const struct __ucontext *);
