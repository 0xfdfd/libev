#ifndef __EV_REQUEST_FORWARD_H__
#define __EV_REQUEST_FORWARD_H__
#ifdef __cplusplus
extern "C" {
#endif

#define EV_READ_INVALID     \
    {\
        EV_LIST_NODE_INIT,/* .node */\
        {/* .data */\
            NULL,                                                   /* .data.bufs */\
            0,                                                      /* .data.nbuf */\
            0,                                                      /* .data.capacity */\
            0,                                                      /* .data.size */\
            { EV_INIT_REPEAT(EV_IOV_MAX, EV_BUF_INIT(NULL, 0)), },  /* .data.bufsml */\
        },\
    }

#define EV_WRITE_INVALID    \
    {\
        EV_LIST_NODE_INIT,                                          /* .node */\
        NULL,                                                       /* .data.bufs */\
        0,                                                          /* .data.nbuf */\
        0,                                                          /* .data.size */\
        0,                                                          /* .data.capacity */\
        { EV_INIT_REPEAT(EV_IOV_MAX, EV_BUF_INIT(NULL, 0)), }       /* .data.bufsml */\
    }

struct ev_read;
typedef struct ev_read ev_read_t;

struct ev_write;
typedef struct ev_write ev_write_t;

#ifdef __cplusplus
}
#endif
#endif
