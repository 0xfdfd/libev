#ifndef __EV_MISC_WIN_INTERNAL_H__
#define __EV_MISC_WIN_INTERNAL_H__

#include "misc.h"

#define EV_FATAL_SYSCALL(errcode, syscall)	\
	ev__fatal_syscall(__FILE__, __LINE__, errcode, syscall)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maps a character string to a UTF-16 (wide character) string.
 * @param[out] dst  Pointer to store wide string. Use #ev_free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
API_LOCAL ssize_t ev__utf8_to_wide(WCHAR** dst, const char* src);

/**
 * @brief Maps a UTF-16 (wide character) string to a character string.
 * @param[out] dst  Pointer to store wide string. Use #ev_free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
API_LOCAL ssize_t ev__wide_to_utf8(char** dst, const WCHAR* src);

API_LOCAL void ev__fatal_syscall(const char* file, int line,
	DWORD errcode, const char* syscall);

#ifdef __cplusplus
}
#endif

#endif
