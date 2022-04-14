#ifndef __EV_REQUEST_H__
#define __EV_REQUEST_H__

#include "ev/backend.h"
#include "ev/handle.h"
#include "ev/buf.h"
#include "ev/request_forward.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read request
 */
struct ev_read
{
    ev_list_node_t          node;               /**< Intrusive node */
    struct
    {
        ev_buf_t*           bufs;               /**< Buffer list */
        size_t              nbuf;               /**< Buffer list count */
        size_t              capacity;           /**< Total bytes of buffer */
        size_t              size;               /**< Data size */
        ev_buf_t            bufsml[EV_IOV_MAX]; /**< Bound buffer list */
    }data;
};

/**
 * @brief Write request
 */
struct ev_write
{
    ev_list_node_t          node;               /**< Intrusive node */
    ev_buf_t*               bufs;               /**< Buffer list */
    size_t                  nbuf;               /**< Buffer list count */
    size_t                  size;               /**< Write size */
    size_t                  capacity;           /**< Total bytes need to send */
    ev_buf_t                bufsml[EV_IOV_MAX]; /**< Bound buffer list */
};

#ifdef __cplusplus
}
#endif

#endif
