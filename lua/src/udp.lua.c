#include "udp.lua.h"
#include "misc.lua.h"

#define LEV_UDP_NAME    "__ev_udp"

typedef struct lev_udp_s
{
    ev_udp_t                sock;
    ev_loop_t*              loop;

    lua_State*              close_L;
    int                     closed;
} lev_udp_t;

typedef struct lev_udp_send
{
    ev_udp_write_t          req;

    lua_State*              L;
    ev_loop_t*              loop;

    ssize_t                 result;
} lev_udp_send_t;

typedef struct lev_udp_recv
{
    ev_udp_read_t           req;
    lua_State*              L;
    ev_loop_t*              loop;

    struct sockaddr_storage storage;

    ssize_t                 result;
    char                    buffer[64 * 1024];
} lev_udp_recv_t;

static void _lev_udp_close_sync(lev_udp_t* udp)
{
    if (!udp->closed)
    {
        udp->closed = 1;
        ev_udp_exit(&udp->sock, NULL);
        ev_loop_run(udp->loop, EV_LOOP_MODE_NOWAIT);
    }
}

static int _lev_udp_gc(lua_State* L)
{
    lev_udp_t* udp = lua_touserdata(L, 1);
    _lev_udp_close_sync(udp);

    return 0;
}

static int _lev_udp_bind(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 2);

    int flags = 0;
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
        flags = (int)lua_tointeger(L, 3);
    }

    if ((ret = ev_udp_bind(&udp->sock, (struct sockaddr*)addr, flags)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    return 0;
}

static int _lev_udp_connect(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    struct sockaddr_storage* addr = lev_to_sockaddr_storage(L, 2);

    if ((ret = ev_udp_connect(&udp->sock, (struct sockaddr*)addr)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    return 0;
}

static int _lev_udp_name(lua_State* L, int(*cb)(ev_udp_t*,struct sockaddr*,size_t*))
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);

    struct sockaddr_storage* addr = lev_push_sockaddr_storage(L);
    size_t addr_len = sizeof(*addr);
    if ((ret = cb(&udp->sock, (struct sockaddr*)addr, &addr_len)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    return 1;
}

static int _lev_udp_getsockname(lua_State* L)
{
    return _lev_udp_name(L, ev_udp_getsockname);
}

static int _lev_udp_getpeername(lua_State* L)
{
    return _lev_udp_name(L, ev_udp_getpeername);
}

static int _lev_udp_set_membership(lua_State* L)
{
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    const char* multicast_addr = luaL_checkstring(L, 2);
    const char* interface_addr = luaL_checkstring(L, 3);
    int membership = (int)luaL_checkinteger(L, 4);

    int ret = ev_udp_set_membership(&udp->sock, multicast_addr, interface_addr, membership);
    if (ret != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    return 0;
}

static int _lev_udp_set_source_membership(lua_State* L)
{
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    const char* multicast_addr = luaL_checkstring(L, 2);
    const char* interface_addr = luaL_checkstring(L, 3);
    const char* source_addr = luaL_checkstring(L, 4);
    int membership = (int)luaL_checkinteger(L, 5);

    int ret = ev_udp_set_source_membership(&udp->sock, multicast_addr,
        interface_addr, source_addr, membership);
    if (ret != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    return 0;
}

static int _lev_udp_set_multicast_loop(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    int on = lua_toboolean(L, 2);

    if ((ret = ev_udp_set_multicast_loop(&udp->sock, on)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    return 0;
}

static int _lev_udp_set_multicast_ttl(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    int ttl = (int)luaL_checkinteger(L, 2);

    if ((ret = ev_udp_set_multicast_ttl(&udp->sock, ttl)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }
    return 0;
}

static int _lev_udp_set_multicast_interface(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    const char* interface_addr = luaL_checkstring(L, 2);

    if ((ret = ev_udp_set_multicast_interface(&udp->sock, interface_addr)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }
    return 0;
}

static int _lev_udp_set_broadcast(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    int on = lua_toboolean(L, 2);

    if ((ret = ev_udp_set_broadcast(&udp->sock, on)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }
    return 0;
}

static int _lev_udp_set_ttl(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    int ttl = (int)luaL_checkinteger(L, 2);

    if ((ret = ev_udp_set_ttl(&udp->sock, ttl)) != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }
    return 0;
}

static void _lev_udp_on_send_done(ev_udp_write_t* req, ssize_t size)
{
    lev_udp_send_t* token = EV_CONTAINER_OF(req, lev_udp_send_t, req);
    token->result = size;

    lev_set_state(token->L, token->loop, 1);
}

static int _lev_udp_on_send_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_udp_send_t* token = (lev_udp_send_t*)ctx;

    if (token->result < 0)
    {
        return lev_error(L, token->loop, (int)token->result, NULL);
    }

    lua_pushinteger(L, token->result);
    return 1;
}

static int _lev_udp_send_template(lua_State* L,
    int(*cb)(ev_udp_t*,ev_udp_write_t*,ev_buf_t*,size_t,const struct sockaddr*, ev_udp_write_cb))
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);

    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

    struct sockaddr_storage* addr = NULL;
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
        addr = lev_to_sockaddr_storage(L, 3);
    }

    lev_udp_send_t* token = lua_newuserdata(L, sizeof(lev_udp_send_t));

    ev_buf_t buf = ev_buf_make((void*)data, data_sz);
    ret = cb(&udp->sock, &token->req, &buf, 1, (struct sockaddr*)addr, _lev_udp_on_send_done);
    if (ret != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    token->L = L;
    token->loop = udp->loop;
    lev_set_state(L, udp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_udp_on_send_resume);
}

static int _lev_udp_send(lua_State* L)
{
    return _lev_udp_send_template(L, ev_udp_send);
}

static int _lev_udp_try_send(lua_State* L)
{
    return _lev_udp_send_template(L, ev_udp_try_send);
}

static void _lev_udp_on_recv_done(ev_udp_read_t* req, const struct sockaddr* addr, ssize_t size)
{
    (void)addr;
    lev_udp_recv_t* token = EV_CONTAINER_OF(req, lev_udp_recv_t, req);
    token->result = size;

    lev_set_state(token->L, token->loop, 1);
}

static int _lev_udp_on_recv_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_udp_recv_t* token = (lev_udp_recv_t*)ctx;

    if (token->result < 0)
    {
        return lev_error(L, token->loop, (int)token->result, NULL);
    }

    lua_pushlstring(L, token->buffer, token->result);
    return 1;
}

static int _lev_udp_recv(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    lev_udp_recv_t* token = lua_newuserdata(L, sizeof(lev_udp_recv_t));

    ev_buf_t buf = ev_buf_make(token->buffer, sizeof(token->buffer));
    ret = ev_udp_recv(&udp->sock, &token->req, &buf, 1, _lev_udp_on_recv_done);
    if (ret != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    token->L = L;
    token->loop = udp->loop;
    lev_set_state(L, udp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_udp_on_recv_resume);
}

static void _lev_copy_sockaddr(struct sockaddr_storage* dst, const struct sockaddr* src)
{
    switch (src->sa_family)
    {
    case AF_INET:
        memcpy(dst, src, sizeof(struct sockaddr_in));
        break;

    case AF_INET6:
        memcpy(dst, src, sizeof(struct sockaddr_in6));
        break;

    default:
        abort();
    }
}

static void _lev_udp_on_recvfrom_done(ev_udp_read_t* req, const struct sockaddr* addr, ssize_t size)
{
    lev_udp_recv_t* token = EV_CONTAINER_OF(req, lev_udp_recv_t, req);
    token->result = size;

    if (token->result >= 0)
    {
        _lev_copy_sockaddr(&token->storage, addr);
    }

    lev_set_state(token->L, token->loop, 1);
}

static int _lev_udp_on_recvfrom_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_udp_recv_t* token = (lev_udp_recv_t*)ctx;

    if (token->result < 0)
    {
        return lev_error(L, token->loop, (int)token->result, NULL);
    }

    lua_pushlstring(L, token->buffer, token->result);

    struct sockaddr_storage* addr = lev_push_sockaddr_storage(L);
    memcpy(addr, &token->storage, sizeof(token->storage));

    return 2;
}

static int _lev_udp_recvfrom(lua_State* L)
{
    int ret;
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);
    lev_udp_recv_t* token = lua_newuserdata(L, sizeof(lev_udp_recv_t));

    ev_buf_t buf = ev_buf_make(token->buffer, sizeof(token->buffer));
    ret = ev_udp_recv(&udp->sock, &token->req, &buf, 1, _lev_udp_on_recvfrom_done);
    if (ret != 0)
    {
        return lev_error(L, udp->loop, ret, NULL);
    }

    token->L = L;
    token->loop = udp->loop;
    lev_set_state(L, udp->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_udp_on_recvfrom_resume);
}

static void _lev_udp_on_close(ev_udp_t* sock)
{
    lev_udp_t* udp = EV_CONTAINER_OF(sock, lev_udp_t, sock);

    lev_set_state(udp->close_L, udp->loop, 1);
    udp->close_L = NULL;
}

static int _lev_udp_close(lua_State* L)
{
    lev_udp_t* udp = luaL_checkudata(L, 1, LEV_UDP_NAME);

    if (udp->closed)
    {
        return 0;
    }

    udp->closed = 1;
    if (!lev_is_yieldable(L, udp->loop))
    {
        _lev_udp_close_sync(udp);
        return 0;
    }

    udp->close_L = L;
    ev_udp_exit(&udp->sock, _lev_udp_on_close);

    return lua_yieldk(L, 0, (lua_KContext)NULL, NULL);
}

static lev_udp_t* lev_make_udp(lua_State* L, ev_loop_t* loop, int domain)
{
    int ret;
    lev_udp_t* udp = lua_newuserdata(L, sizeof(lev_udp_t));

    udp->close_L = NULL;
    udp->closed = 0;
    udp->loop = loop;

    if ((ret = ev_udp_init(loop, &udp->sock, domain)) != 0)
    {
        lev_error(L, loop, ret, NULL);
        return NULL;
    }
    
    static const luaL_Reg s_meta[] = {
        { "__gc",                       _lev_udp_gc },
        { NULL,                         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "bind",                       _lev_udp_bind },
        { "close",                      _lev_udp_close },
        { "connect",                    _lev_udp_connect },
        { "getsockname",                _lev_udp_getsockname },
        { "getpeername",                _lev_udp_getpeername },
        { "recv",                       _lev_udp_recv },
        { "recvfrom",                   _lev_udp_recvfrom },
        { "set_membership",             _lev_udp_set_membership },
        { "set_source_membership",      _lev_udp_set_source_membership },
        { "set_multicast_loop",         _lev_udp_set_multicast_loop },
        { "set_multicast_ttl",          _lev_udp_set_multicast_ttl },
        { "set_multicast_interface",    _lev_udp_set_multicast_interface },
        { "set_broadcast",              _lev_udp_set_broadcast },
        { "set_ttl",                    _lev_udp_set_ttl },
        { "send",                       _lev_udp_send },
        { "try_send",                   _lev_udp_try_send },
        { NULL,                         NULL },
    };
    if (luaL_newmetatable(L, LEV_UDP_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return udp;
}

int lev_udp(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);

    int domain = AF_UNSPEC;
    lev_make_udp(L, loop, domain);

    return 1;
}
