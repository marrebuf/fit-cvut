// Fair assumption.
#define __PIC24FJ256GB106__

// Attributes do us no good, so let's temporarily disable them.
#define __attribute__(x)
#include <p24FJ256GB106.h>
#undef __attribute__

#undef _CONFIG1
#undef _CONFIG2
#undef _CONFIG3

#if !defined WIN32 && !defined __declspec
#define __declspec(x)
#endif

// We have to do it this way.  We can't set an extern value, nor can
// we link against a symbol that may or may not be there.
#define _CONFIG_FUNC(n, x)  __declspec (dllexport) \
	int _return_CONFIG##n (void) { return (x); }

#define _CONFIG1(x)  _CONFIG_FUNC (1, (x))
#define _CONFIG2(x)  _CONFIG_FUNC (2, (x))
#define _CONFIG3(x)  _CONFIG_FUNC (3, (x))

// Rename main() so that the entry points don't clash.
#define main  __main
