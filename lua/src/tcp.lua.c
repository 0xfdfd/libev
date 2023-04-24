#include "tcp.lua.h"
#include "misc.lua.h"
#include <string.h>

#define LEV_TCP_NAME                "__ev_tcp"

typedef struct lev_tcp_s
{
    ev_tcp_t            sock;

    ev_loop_t*          loop;
    lua_State*          L;

    int                 closed;
    lua_State*          close_L;

    int                 ret;
} lev_tcp_t;

typedef struct lev_tcp_write_token
{
    ev_tcp_write_req_t  token;

    lua_State*          L;
    ev_loop_t*          loop;

    ssize_t             ret;
} lev_tcp_write_token_t;

typedef struct lev_tcp_recv_token
{
    ev_tcp_read_req_t   token;

    lua_State*          L;
    ev_loop_t*          loop;

    char                buffer[64 * 1024];
    ssize_t             ret;
} lev_tcp_recv_token_t;

static lev_tcp_t* lev_make_tcp(lua_State* L, ev_loop_t* loop);

static void _lev_tcp_sync_close(lev_tcp_t* tcp)
{
    if (!tcp->closed)
    {
        tcp->closed = 1;
        ev_tcp_exit(&tcp->sock, NULL);
        ev_loop_run(tcp->loop, EV_LOOP_MODE_NOWAIT);
    }
}

static int _lev_tcp_gc(lua_State* L)
{
    lev_tcp_t* tcp = lua_touserdata(L, 1);
    _lev_tcp_sync_close(tcp);
    return 0;
}

static void _lev_tcp_on_connect(ev_tcp_t* sock, int stat)
{
    lev_tcp_t* tcp = EV_CONTAINER_OF(sock, lev_tcp_t, sock);
    tcp->ret = stat;

    lev_set_state(tcp->L, tcp->loop, 1);
}

static int _lev_tcp_on_connect_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_tcp_t* tcp = (lev_tcp_t*)ctx;
    int ret = tcp->ret;

    if (ret < 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

static int _lev_tcp_connect(lua_State* L)
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 2);

    ret = ev_tcp_connect(&tcp->sock, (struct sockaddr*)addr, sizeof(*addr), _lev_tcp_on_connect);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    tcp->L = L;
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)tcp, _lev_tcp_on_connect_resume);
}

static void _lev_tcp_on_write_done(ev_tcp_write_req_t* req, ssize_t size)
{
    lev_tcp_write_token_t* token = EV_CONTAINER_OF(req, lev_tcp_write_token_t, token);

    token->ret = size;
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_tcp_on_write_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_tcp_write_token_t* token = (lev_tcp_write_token_t*)ctx;

    if (token->ret < 0)
    {
        lua_pushinteger(L, token->ret);
        return 1;
    }

    return 0;
}

static int _lev_tcp_send(lua_State* L)
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);

    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

    lev_tcp_write_token_t* token = lua_newuserdata(L, sizeof(lev_tcp_write_token_t));

    ev_buf_t buf = ev_buf_make((void*)data, data_sz);
    ret = ev_tcp_write(&tcp->sock, &token->token, &buf, 1, _lev_tcp_on_write_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->loop = tcp->loop;
    token->L = L;
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_tcp_on_write_resume);
}

static void _lev_tcp_on_read_done(ev_tcp_read_req_t* req, ssize_t size)
{
    lev_tcp_recv_token_t* token = EV_CONTAINER_OF(req, lev_tcp_recv_token_t, token);

    token->ret = size;
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_tcp_on_read_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_tcp_recv_token_t* token = (lev_tcp_recv_token_t*)ctx;

    if (token->ret < 0)
    {
        lua_pushinteger(L, token->ret);
        lua_pushnil(L);
        return 2;
    }

    lua_pushnil(L);
    lua_pushlstring(L, token->buffer, token->ret);
    return 2;
}

static int _lev_tcp_recv(lua_State* L)
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);

    if (tcp->closed)
    {
        lua_pushinteger(L, EV_EPIPE);
        lua_pushnil(L);
        return 2;
    }

    lev_tcp_recv_token_t* token = lua_newuserdata(L, sizeof(lev_tcp_recv_token_t));

    ev_buf_t buf = ev_buf_make(token->buffer, sizeof(token->buffer));
    ret = ev_tcp_read(&tcp->sock, &token->token, &buf, 1, _lev_tcp_on_read_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = tcp->loop;
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_tcp_on_read_resume);
}

static int _lev_tcp_listen(lua_State* L)
{
    int ret = 0;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 2);

    int backlog = 1024;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        backlog = (int)lua_tointeger(L, 3);
    }

    if ((ret = ev_tcp_bind(&tcp->sock, (struct sockaddr*)addr, sizeof(*addr))) != 0)
    {
        goto finish;
    }

    if ((ret = ev_tcp_listen(&tcp->sock, backlog)) != 0)
    {
        goto finish;
    }

finish:
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static void _lev_tcp_on_accept(ev_tcp_t* lisn, ev_tcp_t* conn, int stat)
{
    (void)lisn;
    lev_tcp_t* sock = EV_CONTAINER_OF(conn, lev_tcp_t, sock);
    sock->ret = stat;

    lev_set_state(sock->L, sock->loop, 1);
}

static int _lev_tcp_on_accept_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    int sp = lua_gettop(L);
    lev_tcp_t* sock = (lev_tcp_t*)ctx;
    int ret = sock->ret;

    if (ret == 0)
    {
        lua_pushnil(L);
        lua_insert(L, sp);
        return 2;
    }

    lua_pushinteger(L, ret);
    lua_pushnil(L);

    return 2;
}

static int _lev_tcp_accept(lua_State* L)
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);
    lev_tcp_t* sock = lev_make_tcp(L, tcp->loop);

    if ((ret = ev_tcp_accept(&tcp->sock, &sock->sock, _lev_tcp_on_accept)) != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    sock->L = L;
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)sock, _lev_tcp_on_accept_resume);
}

static int _lev_tcp_name(lua_State* L, int(*cb)(ev_tcp_t*, struct sockaddr*, size_t*))
{
    int ret;
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);

    struct sockaddr_storage* addr = lev_push_sockaddr_storage(L);
    size_t addr_len = sizeof(*addr);
    if ((ret = cb(&tcp->sock, (struct sockaddr*)addr, &addr_len)) != 0)
    {
        return lev_error(L, tcp->loop, ret, NULL);
    }

    return 1;
}

static int _lev_tcp_sockname(lua_State* L)
{
    return _lev_tcp_name(L, ev_tcp_getsockname);
}

static int _lev_tcp_peername(lua_State* L)
{
    return _lev_tcp_name(L, ev_tcp_getpeername);
}

static void _lev_tcp_on_close(ev_tcp_t* sock)
{
    lev_tcp_t* tcp = EV_CONTAINER_OF(sock, lev_tcp_t, sock);

    lev_set_state(tcp->close_L, tcp->loop, 1);
    tcp->close_L = NULL;
}

static int _lev_tcp_close(lua_State* L)
{
    lev_tcp_t* tcp = luaL_checkudata(L, 1, LEV_TCP_NAME);

    if (tcp->closed)
    {
        return 0;
    }

    tcp->closed = 1;
    if (!lev_is_yieldable(L, tcp->loop))
    {
        _lev_tcp_sync_close(tcp);
        return 0;
    }

    tcp->close_L = L;
    ev_tcp_exit(&tcp->sock, _lev_tcp_on_close);
    lev_set_state(L, tcp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)NULL, NULL);
}

static lev_tcp_t* lev_make_tcp(lua_State* L, ev_loop_t* loop)
{
    int  ret;
    lev_tcp_t* tcp = lua_newuserdata(L, sizeof(lev_tcp_t));
    memset(tcp, 0, sizeof(*tcp));

    tcp->loop = loop;
    if ((ret = ev_tcp_init(loop, &tcp->sock)) != 0)
    {
        tcp->closed = 1;
        lev_error(L, loop, ret, NULL);
        return NULL;
    }

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_tcp_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "accept",     _lev_tcp_accept },
        { "close",      _lev_tcp_close },
        { "connect",    _lev_tcp_connect },
        { "listen",     _lev_tcp_listen },
        { "send",       _lev_tcp_send },
        { "recv",       _lev_tcp_recv },
        { "sockname",   _lev_tcp_sockname },
        { "peername",   _lev_tcp_peername },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_TCP_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return tcp;
}

int lev_tcp(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    lev_make_tcp(L, loop);

    return 1;
}
