#ifndef __EV_DEFINES_INTERNAL_H__
#define __EV_DEFINES_INTERNAL_H__

#include "ev/defs.h"
#include "ev/os.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#   include <WS2tcpip.h>
#else
#   include <arpa/inet.h>
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
#   define API_LOCAL    __attribute__((visibility ("hidden")))
#else
#   define API_LOCAL
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

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define EV_MIN(a, b)    ((a) < (b) ? (a) : (b))


/**
 * @brief Align \p size to \p align, who's value is larger or equal to \p size
 *   and can be divided with no remainder by \p align.
 * @note \p align must equal to 2^n
 */
#define ALIGN_SIZE(size, align) \
    (((uintptr_t)(size) + ((uintptr_t)(align) - 1)) & ~((uintptr_t)(align) - 1))

#define ACCESS_ONCE(TYPE, var)  (*(volatile TYPE*) &(var))

/**
 * @brief exchange value of \p v1 and \p v2.
 * @note \p v1 and \p v2 must have the same type.
 * @param[in] TYPE      Type of \p v1 and \p v2.
 * @param[in,out] v1    value1
 * @param[in,out] v2    value2
 */
#define EXCHANGE_VALUE(TYPE, v1, v2)    \
    do {\
        TYPE _tmp = v1;\
        v1 = v2;\
        v2 = _tmp;\
    } while(0)

/**
 * @def EV_COUNT_ARG
 * @brief Count the number of arguments in macro
 */
#ifdef _MSC_VER // Microsoft compilers
#   define EV_COUNT_ARG(...)  _EV_INTERNAL_EXPAND_ARGS_PRIVATE(_EV_INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
/**@cond DOXYGEN_INTERNAL*/
#   define _EV_INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#   define _EV_INTERNAL_EXPAND_ARGS_PRIVATE(...) EAF_EXPAND(_EV_INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#   define _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, count, ...) count
/**@endcond*/
#else // Non-Microsoft compilers
#   define EV_COUNT_ARG(...) _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(0, ## __VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
/**@cond DOXYGEN_INTERNAL*/
#   define _EV_INTERNAL_GET_ARG_COUNT_PRIVATE(_0, _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, count, ...) count
/**@endcond*/
#endif

#define ENSURE_LAYOUT(TYPE_A, FIELD_A_1, FIELD_A_2, TYPE_B, FIELD_B_1, FIELD_B_2)   \
    assert(sizeof(TYPE_A) == sizeof(TYPE_B));\
    assert(offsetof(TYPE_A, FIELD_A_1) == offsetof(TYPE_B, FIELD_B_1));\
    assert(sizeof(((TYPE_A*)0)->FIELD_A_1) == sizeof(((TYPE_B*)0)->FIELD_B_1));\
    assert(offsetof(TYPE_A, FIELD_A_2) == offsetof(TYPE_B, FIELD_B_2));\
    assert(sizeof(((TYPE_A*)0)->FIELD_A_2) == sizeof(((TYPE_B*)0)->FIELD_B_2))

#define EV_ABORT()  ev__abort(__FILE__, __LINE__)

#if defined(__GNUC__) || defined(__clang__)
#   define EV_NORETURN  __attribute__ ((__noreturn__))
#elif defined(_MSC_VER)
#   define EV_NORETURN  __declspec(noreturn)
#else
#   define EV_NORETURN
#endif

#ifdef __cplusplus
}
#endif
#endif
