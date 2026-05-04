// Constructors run before main()

/* Constructors are not in the c standard
 * This implementation requires specific compilers:
 *	gcc
 *	clang
 *	msvc
 */

/* APPLICATION:
 * NORMAL:
 *	void function()
 *	{
 *		// do stuff
 *	}
 *
 * CONSTRUCTOR:
 *	MAKE_CONSTRUCTOR(function)
 *	{
 *		// do stuff
 *	}
 */

#if defined(__GNUC__) || defined(__clang__)

	#define MAKE_CONSTRUCTOR(fn)					\
		__attribute__((constructor))				\
		static void fn()

#elif defined(_MSC_VER)

	#pragma section(".CRT$XCU", read)
	#define MAKE_CONSTRUCTOR(fn)					\
		static void fn();					\
		__declspec(allocate(".CRT$XCU"))			\
		void (*const fn##_constructor)() = fn;			\
		static void fn()

#else

	#error unsupported compiler (use gcc, clang or msvc)

#endif