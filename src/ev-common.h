#ifndef __EV_COMMON_INTERNAL_H__
#define __EV_COMMON_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "ev.h"

#if defined(__GNUC__) || defined(__clang__)
#	define container_of(ptr, type, member) \
		({ \
			const typeof(((type *)0)->member)*__mptr = (ptr); \
			(type *)((char *)__mptr - offsetof(type, member)); \
		})
#else
#	define container_of(ptr, type, member) \
		((type *) ((char *) (ptr) - offsetof(type, member)))
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#if defined(_WIN32)
#	define ABORT()		__debugbreak()
#elif (defined(__clang__) || defined(__GNUC__)) && (defined(__x86_64__) || defined(__i386__))
#	define ABORT()		asm("int3")
#else
#	define ABORT()		*(volatile int*)NULL = 1
#endif

#define ENSURE_LAYOUT(TYPE_A, TYPE_B, FIELD_A_1, FIELD_B_1, FIELD_A_2, FIELD_B_2)	\
	assert(sizeof(TYPE_A) == sizeof(TYPE_B));\
	assert(offsetof(TYPE_A, FIELD_A_1) == offsetof(TYPE_B, FIELD_B_1));\
	assert(sizeof(((TYPE_A*)0)->FIELD_A_1) == sizeof(((TYPE_B*)0)->FIELD_B_1));\
	assert(offsetof(TYPE_A, FIELD_A_2) == offsetof(TYPE_B, FIELD_B_2));\
	assert(sizeof(((TYPE_A*)0)->FIELD_A_2) == sizeof(((TYPE_B*)0)->FIELD_B_2))

typedef enum ev_handle_flag
{
	/* Used by all handles. Bit 0-7. */
	EV_HANDLE_CLOSING		= 0x01 << 0x00,		/**< 1 */
	EV_HANDLE_CLOSED		= 0x01 << 0x01,		/**< 2 */
	EV_HANDLE_ACTIVE		= 0x01 << 0x02,		/**< 4 */

	/* Used by socket */
	EV_TCP_LISTING			= 0x01 << 0x08,		/**< 256 */
	EV_TCP_ACCEPTING		= 0x01 << 0x09,		/**< 512 */
	EV_TCP_STREAMING		= 0x01 << 0x0A,		/**< 1024 */
	EV_TCP_CONNECTING		= 0x01 << 0x0B,		/**< 2048 */
	EV_TCP_BOUND			= 0x01 << 0x0C,		/**< 4096 */
}ev_handle_flag_t;

/**
 * @brief Initialize a handle
 * @param[in] loop		The loop own the handle
 * @param[out] handle	A pointer to the structure
 * @param[in] close_cb	A callback when handle is closed
 */
void ev__handle_init(ev_loop_t* loop, ev_handle_t* handle, ev_close_cb close_cb);

/**
 * @brief Close the handle
 * @note The handle will not closed until close_cb was called, which was given
 *   by #ev__handle_init()
 * @param[in] handle	handler
 */
void ev__handle_exit(ev_handle_t* handle);

/**
 * @brief Set handle as active
 * @param[in] handle	handler
 */
void ev__handle_active(ev_handle_t* handle);

/**
 * @brief Set handle as inactive
 * @param[in] handle	handler
 */
void ev__handle_deactive(ev_handle_t* handle);

/**
 * @brief Check if the handle is in active state
 * @param[in] handle	handler
 * @return				bool
 */
int ev__handle_is_active(ev_handle_t* handle);

/**
 * @brief Check if the handle is in closing or closed state
 * @param[in] handle	handler
 * @return				bool
 */
int ev__handle_is_closing(ev_handle_t* handle);

/**
 * @brief Add a pending task
 * @param[in] loop		Event loop
 * @param[in] token		A pointer to the pending token
 * @param[in] cb		A callback when the pending task is active
 */
void ev__todo(ev_loop_t* loop, ev_todo_t* token, ev_todo_cb cb);

#ifdef __cplusplus
}
#endif
#endif
