#ifndef __EV_DEFINES_INTERNAL_H__
#define __EV_DEFINES_INTERNAL_H__

#include "ev.h"

#if defined(_WIN32)
#   include <WS2tcpip.h>
#else
#   include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(EV_AMALGAMATE_BUILD)
#   if defined(__GNUC__) || defined(__clang__)
#       define API_LOCAL    static __attribute__((unused))
#   else
#       define API_LOCAL    static
#   endif
#elif (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
#   define API_LOCAL    __attribute__((visibility ("hidden")))
#else
#   define API_LOCAL
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

#define EV_JOIN(a, b)   EV_JOIN_2(a, b)
#define EV_JOIN_2(a, b) a##b

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
#   define _EV_INTERNAL_EXPAND_ARGS_PRIVATE(...) EV_EXPAND(_EV_INTERNAL_GET_ARG_COUNT_PRIVATE(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
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

#define EV_ABORT(fmt, ...)  \
    do {\
        fprintf(stderr, "%s:%d:" fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
        abort();\
    } while (0)

#if defined(__GNUC__) || defined(__clang__)
#   define EV_NORETURN  __attribute__ ((__noreturn__))
#elif defined(_MSC_VER)
#   define EV_NORETURN  __declspec(noreturn)
#else
#   define EV_NORETURN
#endif

/**
 * @brief The maximum length for a path.
 * In Windows API, the Ansi version of `MAX_PATH` is defined as 260.
 * The unicode version does not define macro as `MAX_PATH`, but NTFS does
 * support up to 32768 characters in length, but only when using the Unicode
 * APIs.
 *
 * @see https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
 * @see https://docs.microsoft.com/en-us/cpp/c-runtime-library/path-field-limits?view=msvc-170
 */
#if defined(_WIN32)
#   define WIN32_UNICODE_PATH_MAX 32768
#endif

#ifdef __cplusplus
}
#endif
#endif
