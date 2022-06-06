#ifndef __EV_MISC_WIN_INTERNAL_H__
#define __EV_MISC_WIN_INTERNAL_H__

#include "misc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maps a character string to a UTF-16 (wide character) string.
 * @param[out] dst  Pointer to store wide string. Use #ev__free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
API_LOCAL ssize_t ev__utf8_to_wide(WCHAR** dst, const char* src);

/**
 * @brief Maps a UTF-16 (wide character) string to a character string.
 * @param[out] dst  Pointer to store wide string. Use #ev__free() to release it.
 * @param[in] src   Source string.
 * @return          The number of characters (not bytes) of \p dst, or #ev_errno_t if error.
 */
API_LOCAL ssize_t ev__wide_to_utf8(char** dst, const WCHAR* src);

/**
 * @brief Convert typeof NTSTATUS error to typeof WinSock error
 * @param[in] status  NTSTATUS error
 * @return WinSock error
 */
API_LOCAL int ev__ntstatus_to_winsock_error(NTSTATUS status);

#ifdef __cplusplus
}
#endif

#endif
