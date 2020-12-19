#ifndef __EV_COMMON_INTERNAL_H__
#define __EV_COMMON_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#if defined(_WIN32)
#	define ABORT()		__debugbreak(); abort()
#elif (defined(__clang__) || defined(__GNUC__)) && (defined(__x86_64__) || defined(__i386__))
#	define ABORT()		asm("int3"); abort()
#else
#	define ABORT()		*(volatile int*)NULL = 1; abort()
#endif

typedef enum ev_handle_flag
{
	/* Used by all handles. */
	EV_HANDLE_CLOSING	= 0x01 << 0x00,
	EV_HANDLE_CLOSED	= 0x01 << 0x01,
	EV_HANDLE_ACTIVE	= 0x01 << 0x02,
}ev_handle_flag_t;

#ifdef __cplusplus
}
#endif
#endif
