#ifndef __EV_PLATFORM_INTERNAL_H__
#define __EV_PLATFORM_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#   include <WS2tcpip.h>
#else
#   include <arpa/inet.h>
#endif

#include "ev.h"

#if (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
#   define API_LOCAL    __attribute__((visibility ("hidden")))
#   define container_of(ptr, type, member) \
        ({ \
            const typeof(((type *)0)->member)*__mptr = (ptr); \
            (type *)((char *)__mptr - offsetof(type, member)); \
        })
#else
#   define API_LOCAL
#   define container_of(ptr, type, member) \
        ((type *) ((char *) (ptr) - offsetof(type, member)))
#endif

#if defined(_MSC_VER)
#   define BREAK_ABORT()        __debugbreak()
#elif (defined(__clang__) || defined(__GNUC__)) && (defined(__x86_64__) || defined(__i386__))
#   define BREAK_ABORT()        __asm__ volatile("int $0x03")
#elif (defined(__clang__) || defined(__GNUC__)) && defined(__thumb__)
#   define BREAK_ABORT()        __asm__ volatile(".inst 0xde01")
#elif (defined(__clang__) || defined(__GNUC__)) && defined(__arm__) && !defined(__thumb__)
#   define BREAK_ABORT()        __asm__ volatile(".inst 0xe7f001f0")
#elif (defined(__clang__) || defined(__GNUC__)) && defined(__aarch64__) && defined(__APPLE__)
#   define BREAK_ABORT()        __builtin_debugtrap()
#elif (defined(__clang__) || defined(__GNUC__)) && defined(__aarch64__)
#   define BREAK_ABORT()        __asm__ volatile(".inst 0xd4200000")
#elif (defined(__clang__) || defined(__GNUC__)) && defined(__powerpc__)
#   define BREAK_ABORT()        __asm__ volatile(".4byte 0x7d821008")
#elif (defined(__clang__) || defined(__GNUC__)) && defined(__riscv)
#   define BREAK_ABORT()        __asm__ volatile(".4byte 0x00100073")
#else
#   define BREAK_ABORT()        *(volatile int*)NULL = 1
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#   define EV_IPC_FRAME_HDR_MAGIC  (0x48465645)
#elif __BYTE_ORDER == __BIG_ENDIAN
#   define EV_IPC_FRAME_HDR_MAGIC  (0x45564648)
#else
#   error unknown endian
#endif

/**
 * @brief Initialize backend
 * @param[in] loop  loop handler
 * @return          #ev_errno_t
 */
API_LOCAL int ev__loop_init_backend(ev_loop_t* loop);

/**
 * @brief Destroy backend
 * @param[in] loop  loop handler
 */
API_LOCAL void ev__loop_exit_backend(ev_loop_t* loop);

/**
 * @brief Wait for IO event and process
 * @param[in] loop  loop handler
 * @param[in] timeout   timeout in milliseconds
 */
API_LOCAL void ev__poll(ev_loop_t* loop, uint32_t timeout);

/**
 * @brief Translate system error into #ev_errno_t
 * @param[in] syserr    System error
 * @return              #ev_errno_t
 */
API_LOCAL int ev__translate_sys_error(int syserr);

/**
 * @brief Get clocktime
 * @return      Clock time
 */
API_LOCAL uint64_t ev__clocktime(void);

#ifdef __cplusplus
}
#endif
#endif
