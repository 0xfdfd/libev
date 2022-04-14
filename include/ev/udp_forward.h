#ifndef __EV_UDP_FORWARD_H__
#define __EV_UDP_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup EV_UDP UDP
 * @{
 */

enum ev_udp_membership;

/**
 * @brief Typedef of #ev_udp_membership.
 */
typedef enum ev_udp_membership ev_udp_membership_t;

enum ev_udp_flags;

/**
 * @brief Typedef of #ev_udp_flags.
 */
typedef enum ev_udp_flags ev_udp_flags_t;

struct ev_udp;

/**
 * @brief Typedef of #ev_udp.
 */
typedef struct ev_udp ev_udp_t;

struct ev_udp_write;

/**
 * @brief Typedef of #ev_udp_write.
 */
typedef struct ev_udp_write ev_udp_write_t;

struct ev_udp_read;

/**
 * @brief Typedef of #ev_udp_read.
 */
typedef struct ev_udp_read ev_udp_read_t;

/**
 * @brief Callback for #ev_udp_t
 * @param[in] udp   UDP handle
 */
typedef void (*ev_udp_cb)(ev_udp_t* udp);

/**
 * @brief Write callback
 * @param[in] req       Write request
 * @param[in] size      Write size
 * @param[in] stat      Write result
 */
typedef void (*ev_udp_write_cb)(ev_udp_write_t* req, size_t size, int stat);

/**
 * @brief Read callback
 * @param[in] req       Read callback
 * @param[in] size      Read size
 * @param[in] stat      Read result
 */
typedef void (*ev_udp_recv_cb)(ev_udp_read_t* req, size_t size, int stat);

/**
 * @} EV_UDP
 */

#ifdef __cplusplus
}
#endif
#endif
