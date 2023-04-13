#include "promise.lua.h"

#define LEV_PROMISE_NAME    "__ev_promise"

typedef struct lev_promise_value
{
    int                     refkey_sz;
#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4200)
#endif
    int                     refkey[];
#if defined(_MSC_VER)
#   pragma warning(pop)
#endif
} lev_promise_value_t;

struct lev_promise_s
{
    ev_loop_t*              loop;

    lev_promise_value_t*    value;
    ev_list_t               wait_queue;
    ev_list_t               busy_queue;
};

typedef struct lev_promise_wait_token
{
    ev_list_node_t          node;

    lua_State*              wait_L;
    ev_loop_t*              loop;

    lev_promise_cb          cb;
    void*                   data;

    int                     is_busy;
    lev_promise_t*          promise;
} lev_promise_wait_token_t;

static void _lev_promise_drop_value(lua_State* L, lev_promise_t* self)
{
    if (self->value == NULL)
    {
        return;
    }

    size_t i;
    for (i = 0; i < self->value->refkey_sz; i++)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, self->value->refkey[i]);
        self->value->refkey[i] = LUA_NOREF;
    }
    ev_free(self->value);
    self->value = NULL;
}

static void _lev_promise_drop_wait_token(lev_promise_wait_token_t* token)
{
    lev_promise_t* self = token->promise;

    if (token->is_busy)
    {
        ev_list_erase(&self->busy_queue, &token->node);
    }
    else
    {
        ev_list_erase(&self->wait_queue, &token->node);
    }

    ev_free(token);
}

static int _lev_promise_gc(lua_State* L)
{
    ev_list_node_t* it;
    lev_promise_t* self = lua_touserdata(L, 1);

    _lev_promise_drop_value(L, self);

    while ((it = ev_list_begin(&self->wait_queue)) != NULL)
    {
        lev_promise_wait_token_t* token = EV_CONTAINER_OF(it, lev_promise_wait_token_t, node);
        _lev_promise_drop_wait_token(token);
    }

    while ((it = ev_list_begin(&self->busy_queue)) != NULL)
    {
        lev_promise_wait_token_t* token = EV_CONTAINER_OF(it, lev_promise_wait_token_t, node);
        _lev_promise_drop_wait_token(token);
    }

    return 0;
}

static void _lev_promise_wakeup_all(lev_promise_t* self)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&self->wait_queue)) != NULL)
    {
        lev_promise_wait_token_t* token = EV_CONTAINER_OF(it, lev_promise_wait_token_t, node);
        token->is_busy = 1;
        ev_list_push_back(&self->busy_queue, &token->node);
        lev_set_state(token->wait_L, token->loop, 1);
    }
}

static int _lev_promise_set_value(lua_State* L)
{
    lev_promise_t* self = luaL_checkudata(L, 1, LEV_PROMISE_NAME);

    int sp = lua_gettop(L);
    int refkey_sz = sp - 1;

    int ret = lev_promise_set_value(L, self, refkey_sz);
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

static int _lev_promise_push_value(lua_State* L, lev_promise_t* self, lev_promise_cb cb, void* data)
{
    size_t i;
    lev_promise_value_t* value = self->value;
    if (value == NULL)
    {
        return cb(L, self, 0, data);
    }

    for (i = 0; i < value->refkey_sz; i++)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, value->refkey[i]);
    }
    return cb(L, self, value->refkey_sz, data);
}

static int _lev_promise_get_value_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_promise_wait_token_t* token = (lev_promise_wait_token_t*)ctx;
    lev_promise_t* self = token->promise;

    lev_promise_cb cb = token->cb;
    void* data = token->data;

    _lev_promise_drop_wait_token(token);

    return _lev_promise_push_value(L, self, cb, data);
}

static int _lev_promise_on_get_value(lua_State* L, lev_promise_t* promise, int narg, void* data)
{
    (void)L; (void)promise; (void)data;
    return narg;
}

static int _lev_promise_get_value(lua_State* L)
{
    lev_promise_t* self = luaL_checkudata(L, 1, LEV_PROMISE_NAME);
    return lev_promise_get_value(L, self, _lev_promise_on_get_value, NULL);
}

int lev_promise_get_value(lua_State* L, lev_promise_t* promise, lev_promise_cb cb, void* data)
{
    if (promise->value != NULL)
    {
        return _lev_promise_push_value(L, promise, cb, data);
    }

    lev_promise_wait_token_t* token = ev_malloc(sizeof(lev_promise_wait_token_t));
    if (token == NULL)
    {
        return lev_error(L, promise->loop, EV_ENOMEM, NULL);
    }

    token->is_busy = 0;
    token->wait_L = L;
    token->loop = promise->loop;
    token->promise = promise;
    token->cb = cb;
    token->data = data;

    ev_list_push_back(&promise->wait_queue, &token->node);
    lev_set_state(L, promise->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)token, _lev_promise_get_value_resume);
}

lev_promise_t* lev_promise_make(lua_State* L, ev_loop_t* loop)
{
    lev_promise_t* self = lua_newuserdata(L, sizeof(lev_promise_t));

    self->loop = loop;
    self->value = NULL;
    ev_list_init(&self->wait_queue);
    ev_list_init(&self->busy_queue);

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_promise_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        { "set_value",  _lev_promise_set_value },
        { "get_value",  _lev_promise_get_value },
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_PROMISE_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return self;
}

int lev_promise(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    lev_promise_make(L, loop);

    return 1;
}

int lev_promise_set_value(lua_State* L, lev_promise_t* promise, int narg)
{
    if (promise->value != NULL)
    {
        return EV_EEXIST;
    }

    size_t malloc_size = sizeof(lev_promise_value_t) + sizeof(int) * narg;
    if ((promise->value = ev_malloc(malloc_size)) == NULL)
    {
        return lev_error(L, promise->loop, EV_ENOMEM, NULL);
    }

    /* Save value */
    promise->value->refkey_sz = narg;
    for (; narg > 0; narg--)
    {
        promise->value->refkey[narg - 1] = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    _lev_promise_wakeup_all(promise);

    return 0;
}
