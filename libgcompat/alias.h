#ifndef _ALIAS_H_
#define _ALIAS_H_

#define alias(old, new)                                                        \
	extern __typeof(old) new __attribute__((__alias__(#old)))
#ifndef weak_alias
#define weak_alias(old, new)                                                   \
	extern __typeof(old) new __attribute__((weak, __alias__(#old)))
#endif
#endif /* _ALIAS_H_ */
