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

EV_LOCAL int ev__translate_posix_sys_error(int syserr);

#ifdef __cplusplus
}
#endif
#endif
