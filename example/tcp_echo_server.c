/**
 * @file
 */
#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#define CHECK_SUCCESS(ret) \
    do {\
        int _ret = ret;\
        if (_ret == 0) {\
            break;\
        }\
        fprintf(stderr, "%s:%d: %s\n",\
            __FUNCTION__, __LINE__, ev_strerror(ret));\
    } while (0)

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

typedef struct tcp_client
{
    ev_list_node_t      node;           /* list node */
    ev_tcp_t            client;         /* TCP connecting token */
    ev_tcp_read_req_t   read_req;       /* Read token */
    ev_tcp_write_req_t  write_req;      /* Write token */
    char                buffer[1024];   /* Receive / Send buffer */
    char                ip[64];         /* Peer IP */
    int                 port;           /* Peer port */
} tcp_client_t;

typedef struct tcp_echo_server_s
{
    ev_loop_t           loop;           /* Event loop */
    ev_async_t          on_sigint;      /* Async handle for signal */
    ev_tcp_t            sock_listen;    /* Listing socket */
    ev_list_t           client_list;    /* List of client connections */
    const char*         addr;           /* Listening IP address */
    const char*         port;           /* Listening port */
}tcp_echo_server_t;

static tcp_echo_server_t s_tcp_echo_server = {
    EV_LOOP_INVALID,
    EV_ASYNC_INVALID,
    EV_TCP_INVALID,
    EV_LIST_INIT,
    "0.0.0.0",
    NULL,
};

#if defined(SIGUSR1) && defined(SIGUSR1)
#   define TERM_SIGNAL_MAP(XX) \
        XX(SIGINT)\
        XX(SIGTERM)\
        XX(SIGUSR1)\
        XX(SIGUSR1)
#else
#   define TERM_SIGNAL_MAP(XX) \
        XX(SIGINT)\
        XX(SIGTERM)
#endif

#define EXPAND_SIG_AS_ARRAY(SIG) SIG,
static int s_term_signal[] = {
    TERM_SIGNAL_MAP(EXPAND_SIG_AS_ARRAY)
};
#undef EXPAND_SIG_AS_ARRAY

#define EXPAND_SIG_AS_HELP(SIG) "  + " #SIG "\n"
static const char* s_help =
"USAGE: tcp_echo_server --addr=ADDR --port=PORT\n"
"\n"
"Mandatory arguments to long options are mandatory for short options too.\n"
"  --addr=ADDR\n"
"      Set listen address.\n"
"  --port=PORT\n"
"      Set listen port.\n"
"  --help, -h\n"
"      Show this help and exit.\n"
"\n"
"To exit normally, send one of following signals to this process:\n"
TERM_SIGNAL_MAP(EXPAND_SIG_AS_HELP)
;
#undef EXPAND_SIG_AS_HELP

static void _on_accept_done(ev_tcp_t* lisn, ev_tcp_t* conn, int stat);
static void _want_read(tcp_client_t* client);

static void _setup_args(int argc, char* argv[])
{
    const char* opt = "";

    int i;
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--help") == 0
            || strcmp(argv[i], "-h") == 0)
        {
            goto print_help_and_exit;
        }

        opt = "--addr=";
        if(strncmp(argv[i], opt, strlen(opt)) == 0)
        {
            s_tcp_echo_server.addr = argv[i] + strlen(opt);
            continue;
        }

        opt = "--port=";
        if (strncmp(argv[i], opt, strlen(opt)) == 0)
        {
            s_tcp_echo_server.port = argv[i] + strlen(opt);
            continue;
        }
    }

    if (s_tcp_echo_server.port == NULL)
    {
        goto print_help_and_exit;
    }

    return;

print_help_and_exit:
    printf("%s", s_help);
    exit(EXIT_SUCCESS);
}

static void _on_client_close(ev_tcp_t* sock)
{
    tcp_client_t* client = EV_CONTAINER_OF(sock, tcp_client_t, client);
    printf("connection(%s:%d) teardown\n", client->ip, client->port);

    free(client);
}

static void submit_accept_request(void)
{
    int ret;
    tcp_client_t* client = calloc(1, sizeof(tcp_client_t));
    ret = ev_tcp_init(&s_tcp_echo_server.loop, &client->client);
    CHECK_SUCCESS(ret);
    ev_list_push_back(&s_tcp_echo_server.client_list, &client->node);

    /*
     * Here we try to accept one new connect.
     *
     * After this function success, only one new connect will be accepted,
     * other incoming connections will be in accept queue.
     *
     * In short, one #ev_tcp_accept() call, one connection.
     */
    ret = ev_tcp_accept(&s_tcp_echo_server.sock_listen, &client->client, _on_accept_done);
    CHECK_SUCCESS(ret);
}

static void _close_client(tcp_client_t* client)
{
    ev_list_erase(&s_tcp_echo_server.client_list, &client->node);
    ev_tcp_exit(&client->client, _on_client_close);
}

static void _on_write_done(ev_tcp_write_req_t* req, size_t size, int stat)
{
    (void)size;
    tcp_client_t* client = EV_CONTAINER_OF(req, tcp_client_t, write_req);

    if (stat != 0)
    {
        _close_client(client);
        return;
    }

    _want_read(client);
}

static void _on_read_done(ev_tcp_read_req_t* req, size_t size, int stat)
{
    int ret;
    tcp_client_t* client = EV_CONTAINER_OF(req, tcp_client_t, read_req);

    if (stat != 0)
    {
        _close_client(client);
        return;
    }

    ev_buf_t buf = ev_buf_make(client->buffer, size);
    ret = ev_tcp_write(&client->client, &client->write_req, &buf, 1, _on_write_done);
    CHECK_SUCCESS(ret);
}

static void _want_read(tcp_client_t* client)
{
    ev_buf_t buf = ev_buf_make(client->buffer, sizeof(client->buffer));
    int ret = ev_tcp_read(&client->client, &client->read_req, &buf, 1, _on_read_done);
    CHECK_SUCCESS(ret);
}

static void _show_connect_info(tcp_client_t* client)
{
    int ret;

    struct sockaddr_storage sock_addr;
    size_t sock_addr_len = sizeof(sock_addr);
    ret = ev_tcp_getpeername(&client->client, (struct sockaddr*)&sock_addr, &sock_addr_len);
    CHECK_SUCCESS(ret);

    ret = ev_ipv4_name((struct sockaddr_in*)&sock_addr, &client->port, client->ip, sizeof(client->ip));
    CHECK_SUCCESS(ret);

    printf("connection(%s:%d) setup\n", client->ip, client->port);
}

static void _on_accept_done(ev_tcp_t* lisn, ev_tcp_t* conn, int stat)
{
    assert(lisn == &s_tcp_echo_server.sock_listen); (void)lisn;

    if (stat != 0)
    {
        return;
    }

    tcp_client_t* client = EV_CONTAINER_OF(conn, tcp_client_t, client);
    _show_connect_info(client);

    /* No matter what result it is, we want to accept more connections. */
    submit_accept_request();

    /* If accept failed, destroy resource */
    if (stat != 0)
    {
        _close_client(client);
        return;
    }

    _want_read(client);
}

static void _at_exit(void)
{
    /*
     * At this point, the means the #main() function has returned, and all
     * ev handles except #s_tcp_echo_server::loop have been closed.
     *
     * So close event loop, so that all resource is new released.
     */
    ev_loop_exit(&s_tcp_echo_server.loop);
}

static void _close_listen_socket()
{
    ev_tcp_exit(&s_tcp_echo_server.sock_listen, NULL);
}

static void _close_all_client(void)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&s_tcp_echo_server.client_list)) != NULL)
    {
        tcp_client_t* client = EV_CONTAINER_OF(it, tcp_client_t, node);
        ev_tcp_exit(&client->client, _on_client_close);
    }
}

static void _at_sigint(int signum)
{
    (void)signum;
    ev_async_wakeup(&s_tcp_echo_server.on_sigint);
}

static void _exit_application(void)
{
    _close_listen_socket();
    _close_all_client();
    ev_async_exit(&s_tcp_echo_server.on_sigint, NULL);
}

static void _on_async_sigint(ev_async_t* async)
{
    (void)async;
    _exit_application();
}

int main(int argc, char* argv[])
{
    int ret = 0;

    atexit(_at_exit);

    /*
     * Stop program by one of:
     * + pressing Ctrl-C
     * + `kill -s SIGTERM PID`
     * + `kill -s SIGUSR1 PID`
     * + `kill -s SIGUSR2 PID`
     */
    for (ret = 0; (unsigned long)ret < ARRAY_SIZE(s_term_signal); ret++)
    {
        signal(s_term_signal[ret], _at_sigint);
    }

    /* Parser listen address and port */
    _setup_args(argc, argv);

    /* Initialize event loop */
    ret = ev_loop_init(&s_tcp_echo_server.loop);
    CHECK_SUCCESS(ret);

    /* Initialize async handler */
    ret = ev_async_init(&s_tcp_echo_server.loop, &s_tcp_echo_server.on_sigint, _on_async_sigint);
    CHECK_SUCCESS(ret);

    /* Initialize listen socket */
    ret = ev_tcp_init(&s_tcp_echo_server.loop, &s_tcp_echo_server.sock_listen);
    CHECK_SUCCESS(ret);

    /* Convert listen address to system struct */
    struct sockaddr_storage listen_addr;
    if (strchr(s_tcp_echo_server.addr, ':') != NULL)
    {
        ret = ev_ipv6_addr(s_tcp_echo_server.addr, atoi(s_tcp_echo_server.port),
                           (struct sockaddr_in6*)&listen_addr);
    }
    else
    {
        ret = ev_ipv4_addr(s_tcp_echo_server.addr, atoi(s_tcp_echo_server.port),
                           (struct sockaddr_in*)&listen_addr);
    }
    CHECK_SUCCESS(ret);

    /* Bind listen address */
    ret = ev_tcp_bind(&s_tcp_echo_server.sock_listen,
        (struct sockaddr*)&listen_addr, sizeof(listen_addr));
    CHECK_SUCCESS(ret);

    /*
     * Start listen.
     * Do remember you have to call #ev_tcp_accept() to asynchronously accept
     * new connections.
     */
    ret = ev_tcp_listen(&s_tcp_echo_server.sock_listen, 1024);
    CHECK_SUCCESS(ret);

    /* Try to accept **one** new connection */
    submit_accept_request();

    /*
     * All of above operations is a `request`, you have to call #ev_loop_run()
     * to actually do it.
     *
     * With #EV_LOOP_MODE_DEFAULT set, after all operations are done and no
     * pending request left, #ev_loop_run() will return.
     *
     * In this application, this means:
     * 1. all client connections is closed.
     * 2. listing socket is closed.
     * 3. async handle is closed.
     */
    return ev_loop_run(&s_tcp_echo_server.loop, EV_LOOP_MODE_DEFAULT);
}
