#ifndef __EV_ASSERT_INTERNAL_H__
#define __EV_ASSERT_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Assertion macro.
 * 
 * If \p exp is false, this macro prints an error message to standard error and
 * terminates the program by calling abort().
 * 
 * User can optionally add custom message to this macro just like printf(). In
 * this case, use this macro as `EV_ASSERT(exp, fmt, ...)`.
 */
#define EV_ASSERT(exp, ...)   EV_JOIN(_EV_ASSERT_, EV_BARG(__VA_ARGS__))(exp, ##__VA_ARGS__)

/**
 * @brief Helper assertion macro.
 *
 * #EV_ASSERT() will expand to this if no print format exist.
 */
#define _EV_ASSERT_0(exp, ...) \
    do {\
        int _assert_ret = (exp);\
        if (_assert_ret) {\
            break;\
        }\
        ev__assertion_failure(#exp, __FILE__, __LINE__, NULL);\
    } while (0)

/**
 * @brief Helper assertion macro.
 *
 * #EV_ASSERT() will expand to this if print format exist.
 */
#define _EV_ASSERT_1(exp, fmt, ...)    \
    do {\
        int _assert_ret = (exp);\
        if (_assert_ret) {\
            break;\
        }\
        ev__assertion_failure(#exp, __FILE__, __LINE__, fmt, ##__VA_ARGS__);\
    } while (0)

/**
 * @brief Assertion failure function.
 * @warning Do NOT use this function directly. Use #EV_ASSERT() instead.
 * @param[in] exp   The expression that failed.
 * @param[in] file  The file that \p exp located.
 * @param[in] line  The line that \p exp located.
 * @param[in] fmt   Custom print format. Set to NULL if no user print format.
 * @param[in] ...   Variable list for \p fmt.
 */
EV_LOCAL void ev__assertion_failure(const char* exp, const char* file, int line, const char* fmt, ...);

#ifdef __cplusplus
}
#endif
#endif
