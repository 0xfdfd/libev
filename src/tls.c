#include "ev.h"

int ev_tls_init(ev_loop_t* loop, ev_tls_t* tls)
{
    int ret;

    if ((ret = ev_tcp_init(loop, &tls->tcp)) != 0)
    {
        return ret;
    }

    tls->on_exit = NULL;
    return 0;
}

static void _ev_tls_on_tcp_close(ev_tcp_t* sock)
{
    ev_tls_t* tls = EV_CONTAINER_OF(sock, ev_tls_t, tcp);
    if (tls->on_exit != NULL)
    {
        tls->on_exit(tls);
    }
}

void ev_tls_exit(ev_tls_t* tls, ev_tls_cb on_exit)
{
    tls->on_exit = on_exit;
    ev_tcp_exit(&tls->tcp, _ev_tls_on_tcp_close);
}
