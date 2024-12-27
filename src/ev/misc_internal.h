#ifndef __EV_MISC_INTERNAL_H__
#define __EV_MISC_INTERNAL_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Translate system error into #ev_errno_t
 * @param[in] syserr    System error
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__translate_sys_error(int syserr);

/**
 * @brief Translate posix error into #ev_errno_t
 * @param[in] syserr    Posix error.
 * @return              #ev_errno_t
 */
EV_LOCAL int ev__translate_posix_sys_error(int syserr);

/**
 * @brief Fill \p buf with random data.
 * @param[out] buf  The buffer to fill.
 * @param[in] len   The number of bytes to fill.
 * @return          #ev_errno_t
 */
EV_LOCAL int ev__random(void* buf, size_t len);

EV_LOCAL void ev__backend_shutdown(void);

#ifdef __cplusplus
}
#endif
#endif
