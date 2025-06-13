#include <fenv.h>
int fedisableexcept(int excepts) {
	return 0;
}

int
feenableexcept (int excepts)
{
  unsigned short int new_exc, old_exc;
  unsigned int new;

  excepts &= FE_ALL_EXCEPT;

  /* Get the current control word of the x87 FPU.  */
  __asm__ ("fstcw %0" : "=m" (*&new_exc));

  old_exc = (~new_exc) & FE_ALL_EXCEPT;

  new_exc &= ~excepts;
  __asm__ ("fldcw %0" : : "m" (*&new_exc));

  /* And now the same for the SSE MXCSR register.  */
  __asm__ ("stmxcsr %0" : "=m" (*&new));

  /* The SSE exception masks are shifted by 7 bits.  */
  new &= ~(excepts << 7);
  __asm__ ("ldmxcsr %0" : : "m" (*&new));

  return old_exc;
}

int
fegetexcept (void)
{
  unsigned short int exc;

  /* Get the current control word.  */
  __asm__ ("fstcw %0" : "=m" (*&exc));

  return (~exc) & FE_ALL_EXCEPT;
}
