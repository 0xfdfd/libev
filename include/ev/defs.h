/**
 * @file
 */
#ifndef __EV_DEFINES_H__
#define __EV_DEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The maximum number of iov buffers can be support.
 */
#define EV_IOV_MAX              16

/**
 * @brief Expand macro.
 * @param[in] ...   Code to expand.
 */
#define EV_EXPAND(...)          __VA_ARGS__

/**
 * @brief Repeat code for \p n times.
 * @param[in] n     How many times to repeat.
 * @param[in] ...   Code fragment to repeat.
 */
#define EV_INIT_REPEAT(n, ...)   EV_INIT_REPEAT2(n, __VA_ARGS__)
/** @cond */
#define EV_INIT_REPEAT2(n, ...)  EV_INIT_REPEAT_##n(__VA_ARGS__)
#define EV_INIT_REPEAT_1(...)    EV_EXPAND(__VA_ARGS__)
#define EV_INIT_REPEAT_2(...)    EV_INIT_REPEAT_1(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_3(...)    EV_INIT_REPEAT_2(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_4(...)    EV_INIT_REPEAT_3(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_5(...)    EV_INIT_REPEAT_4(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_6(...)    EV_INIT_REPEAT_5(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_7(...)    EV_INIT_REPEAT_6(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_8(...)    EV_INIT_REPEAT_7(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_9(...)    EV_INIT_REPEAT_8(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_10(...)   EV_INIT_REPEAT_9(__VA_ARGS__),  EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_11(...)   EV_INIT_REPEAT_10(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_12(...)   EV_INIT_REPEAT_11(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_13(...)   EV_INIT_REPEAT_12(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_14(...)   EV_INIT_REPEAT_13(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_15(...)   EV_INIT_REPEAT_14(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
#define EV_INIT_REPEAT_16(...)   EV_INIT_REPEAT_15(__VA_ARGS__), EV_INIT_REPEAT_1(__VA_ARGS__)
/** @endcond */

/**
 * @def EV_CONTAINER_OF
 * @brief cast a member of a structure out to the containing structure.
 */
#if defined(container_of)
#   define EV_CONTAINER_OF(ptr, type, member)   \
        container_of(ptr, type, member)
#elif defined(__GNUC__) || defined(__clang__)
#   define EV_CONTAINER_OF(ptr, type, member)   \
        ({ \
            const typeof(((type *)0)->member)*__mptr = (ptr); \
            (type *)((char *)__mptr - offsetof(type, member)); \
        })
#else
#   define EV_CONTAINER_OF(ptr, type, member)   \
        ((type *) ((char *) (ptr) - offsetof(type, member)))
#endif

#ifdef __cplusplus
}
#endif
#endif
