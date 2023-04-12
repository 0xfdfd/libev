#include "pipe.lua.h"

#define LEV_PIPE_NAME   "__ev_pipe"

typedef struct lev_pipe_s
{
    ev_pipe_t           pipe;

    ev_loop_t*          loop;
    lua_State*          L;

    int                 closed;
    lua_State*          close_L;
} lev_pipe_t;

typedef struct lev_pipe_send_token
{
    ev_pipe_write_req_t token;

    lua_State*          L;
    ev_loop_t*          loop;

    ssize_t             ret;
} lev_pipe_send_token_t;

typedef struct lev_pipe_recv_token
{
    ev_pipe_read_req_t  token;

    lua_State*          L;
    ev_loop_t*          loop;

    size_t              cap;
    ssize_t             ret;
#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    char                buffer[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
} lev_pipe_recv_token_t;

static int _lev_pipe_sync_close(lev_pipe_t* self)
{
    if (!self->closed)
    {
        self->closed = 1;
        ev_pipe_exit(&self->pipe, NULL);
        ev_loop_run(self->loop, EV_LOOP_MODE_NOWAIT);
    }
    return 0;
}

static int _lev_pipe_gc(lua_State* L)
{
    lev_pipe_t* self = lua_touserdata(L, 1);
    return _lev_pipe_sync_close(self);
}

static void _lev_pipe_on_exit(ev_pipe_t* handle)
{
    lev_pipe_t* self = EV_CONTAINER_OF(handle, lev_pipe_t, pipe);

    lev_set_state(self->close_L, self->loop, 1);
    self->close_L = NULL;
}

static int _lev_pipe_close(lua_State* L)
{
    lev_pipe_t* self = luaL_checkudata(L, 1, LEV_PIPE_NAME);

    if (self->closed)
    {
        return 0;
    }

    self->closed = 1;
    if (!lev_is_yieldable(L, self->loop))
    {
        return _lev_pipe_sync_close(self);
    }

    self->close_L = L;
    ev_pipe_exit(&self->pipe, _lev_pipe_on_exit);
    lev_set_state(L, self->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)NULL, NULL);
}

static void _lev_pipe_on_send_done(ev_pipe_write_req_t* req, ssize_t size)
{
    lev_pipe_send_token_t* token = EV_CONTAINER_OF(req, lev_pipe_send_token_t, token);

    token->ret = size;
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_pipe_on_send_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_pipe_send_token_t* token = (lev_pipe_send_token_t*)ctx;

    if (token->ret < 0)
    {
        lua_pushinteger(L, token->ret);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

static int _lev_pipe_send(lua_State* L)
{
    int ret;
    lev_pipe_t* self = luaL_checkudata(L, 1, LEV_PIPE_NAME);
    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

    lev_pipe_send_token_t* token = lua_newuserdata(L, sizeof(lev_pipe_send_token_t));

    ev_buf_t buf = ev_buf_make((void*)data, data_sz);
    ret = ev_pipe_write(&self->pipe, &token->token, &buf, 1, _lev_pipe_on_send_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        return 1;
    }

    token->L = L;
    token->loop = self->loop;
    token->ret = 0;
    lev_set_state(L, self->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_pipe_on_send_resume);
}

static void _lev_pipe_on_recv_done(ev_pipe_read_req_t* req, ssize_t result)
{
    lev_pipe_recv_token_t* token = EV_CONTAINER_OF(req, lev_pipe_recv_token_t, token);

    token->ret = result;
    lev_set_state(token->L, token->loop, 1);
}

static int _lev_pipe_on_recv_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_pipe_recv_token_t* token = (lev_pipe_recv_token_t*)ctx;

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

static int _lev_pipe_recv(lua_State* L)
{
    int ret;
    lev_pipe_t* self = luaL_checkudata(L, 1, LEV_PIPE_NAME);

    size_t cap = 64 * 1024;
    if (lua_type(L, 2) == LUA_TNUMBER)
    {
        cap = lua_tointeger(L, 2);
    }

    size_t malloc_size = sizeof(lev_pipe_recv_token_t) + cap;
    lev_pipe_recv_token_t* token = lua_newuserdata(L, malloc_size);
    token->cap = cap;

    ev_buf_t buf = ev_buf_make(token->buffer, token->cap);
    ret = ev_pipe_read(&self->pipe, &token->token, &buf, 1, _lev_pipe_on_recv_done);
    if (ret != 0)
    {
        lua_pushinteger(L, ret);
        lua_pushnil(L);
        return 2;
    }

    token->L = L;
    token->loop = self->loop;
    lev_set_state(L, self->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_pipe_on_recv_resume);
}

static lev_pipe_t* _lev_make_pipe(lua_State* L, ev_loop_t* loop)
{
    int ret;
    lev_pipe_t* self = lua_newuserdata(L, sizeof(lev_pipe_t));
    memset(self, 0, sizeof(*self));

    self->loop = loop;
    if ((ret = ev_pipe_init(loop, &self->pipe, 0)) != 0)
    {
        lev_error(L, loop, ret, NULL);
        return NULL;
    }

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_pipe_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "send",       _lev_pipe_send },
        { "recv",       _lev_pipe_recv },
        { "close",      _lev_pipe_close },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_PIPE_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return self;
}

int lev_pipe(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);

    _lev_make_pipe(L, loop);

    return 1;
}

ev_pipe_t* lev_try_to_pipe(lua_State* L, int idx)
{
    lev_pipe_t* self = luaL_testudata(L, idx, LEV_PIPE_NAME);
    return self != NULL ? &self->pipe : NULL;
}
