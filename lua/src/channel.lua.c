#include "channel.lua.h"
#include <string.h>

#define LEV_CHANNEL_NAME    "__ev_channel"

typedef struct lev_channel_s
{
    ev_loop_t*      loop;
    int             is_closed;

    ev_list_t       data_queue;
    ev_list_t       wait_queue;
    ev_list_t       busy_queue;
} lev_channel_t;

typedef struct lev_channel_data_token
{
    ev_list_node_t  node;

    size_t          refkey_sz;
#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    int             refkey[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
} lev_channel_data_token_t;

typedef struct lev_channel_wait_token
{
    ev_list_node_t  node;

    lev_channel_t*  channel;
    lua_State*      wait_L;

    int             is_busy;
} lev_channel_wait_token_t;

static void _lev_channel_drop_data(lua_State* L, lev_channel_data_token_t* token)
{
    size_t i;
    for (i = 0; i < token->refkey_sz; i++)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, token->refkey[i]);
        token->refkey[i] = LUA_NOREF;
    }

    ev_free(token);
}

static void _lev_channel_drop_wait(lua_State* L, lev_channel_wait_token_t* wait_token)
{
    (void)L;

    lev_channel_t* self = wait_token->channel;
    wait_token->channel = NULL;
    wait_token->wait_L = NULL;

    if (wait_token->is_busy)
    {
        ev_list_erase(&self->busy_queue, &wait_token->node);
    }
    else
    {
        ev_list_erase(&self->wait_queue, &wait_token->node);
    }

    ev_free(wait_token);
}

static int _lev_channel_gc(lua_State* L)
{
    ev_list_node_t* it;
    lev_channel_t* self = lua_touserdata(L, 1);

    while ((it = ev_list_pop_front(&self->data_queue)) != NULL)
    {
        lev_channel_data_token_t* data_token = EV_CONTAINER_OF(it, lev_channel_data_token_t, node);
        _lev_channel_drop_data(L, data_token);
    }

    while ((it = ev_list_pop_front(&self->wait_queue)) != NULL)
    {
        lev_channel_wait_token_t* wait_token = EV_CONTAINER_OF(it, lev_channel_wait_token_t, node);
        _lev_channel_drop_wait(L, wait_token);
    }

    while ((it = ev_list_pop_front(&self->busy_queue)) != NULL)
    {
        lev_channel_wait_token_t* wait_token = EV_CONTAINER_OF(it, lev_channel_wait_token_t, node);
        _lev_channel_drop_wait(L, wait_token);
    }

    return 0;
}

static int _lev_channel_send(lua_State* L)
{
    lev_channel_t* self = luaL_checkudata(L, 1, LEV_CHANNEL_NAME);

    if (self->is_closed)
    {
        lua_pushinteger(L, EV_EPIPE);
        return 1;
    }

    int sp = lua_gettop(L);

    size_t refkey_sz = sp - 1;
    size_t malloc_size = sizeof(lev_channel_data_token_t) + sizeof(int) * refkey_sz;

    /* Save data */
    lev_channel_data_token_t* data_token = ev_malloc(malloc_size);
    data_token->refkey_sz = refkey_sz;
    for (; sp > 1; sp--)
    {
        data_token->refkey[sp - 2] = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    ev_list_push_back(&self->data_queue, &data_token->node);

    /* Wakeup one */
    ev_list_node_t* node = ev_list_pop_front(&self->wait_queue);
    if (node == NULL)
    {
        goto finish;
    }

    lev_channel_wait_token_t* wait_token = EV_CONTAINER_OF(node, lev_channel_wait_token_t, node);
    lev_set_state(wait_token->wait_L, self->loop, 1);
    ev_list_push_back(&self->busy_queue, &wait_token->node);

finish:
    lua_pushnil(L);
    return 1;
}

static int _lev_channel_process_recv(lua_State* L, lev_channel_t* self)
{
    ev_list_node_t* it = ev_list_pop_front(&self->data_queue);
    if (it != NULL)
    {
        lua_pushnil(L);

        lev_channel_data_token_t* data_token = EV_CONTAINER_OF(it, lev_channel_data_token_t, node);
        size_t refkey_sz = data_token->refkey_sz;

        size_t i;
        for (i = 0; i < data_token->refkey_sz; i++)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, data_token->refkey[i]);
            luaL_unref(L, LUA_REGISTRYINDEX, data_token->refkey[i]);
            data_token->refkey[i] = LUA_NOREF;
        }

        ev_free(data_token);
        return 1 + (int)refkey_sz;
    }

    if (self->is_closed)
    {
        lua_pushnumber(L, EV_EOF);
        return 1;
    }

    return lev_error(L, self->loop, EV_EPIPE, "no data");
}

static int _lev_channel_recv_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)status;
    lev_channel_wait_token_t* wait_token = (lev_channel_wait_token_t*)ctx;
    lev_channel_t* self = wait_token->channel;

    int ret = _lev_channel_process_recv(L, self);

    _lev_channel_drop_wait(L, wait_token);

    return ret;
}

static int _lev_channel_recv(lua_State* L)
{
    int ret;
    lev_channel_t* self = luaL_checkudata(L, 1, LEV_CHANNEL_NAME);

    if ((ret = _lev_channel_process_recv(L, self)) != 0)
    {
        return ret;
    }

    lev_channel_wait_token_t* wait_token = ev_malloc(sizeof(lev_channel_wait_token_t));
    wait_token->wait_L = L;
    wait_token->channel = self;
    wait_token->is_busy = 0;
    ev_list_push_back(&self->wait_queue, &wait_token->node);
    lev_set_state(L, self->loop, 0);

    return lua_yieldk(L, 0, (lua_KContext)wait_token, _lev_channel_recv_resume);
}

static void _lev_channel_wakeup_all(lev_channel_t* self)
{
    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&self->wait_queue)) != NULL)
    {
        lev_channel_wait_token_t* token = EV_CONTAINER_OF(it, lev_channel_wait_token_t, node);

        token->is_busy = 1;
        ev_list_push_back(&self->busy_queue, &token->node);
        lev_set_state(token->wait_L, token->channel->loop, 1);
    }
}

static int _lev_channel_close(lua_State* L)
{
    lev_channel_t* self = luaL_checkudata(L, 1, LEV_CHANNEL_NAME);

    self->is_closed = 1;
    _lev_channel_wakeup_all(self);

    return 0;
}

static lev_channel_t* _lev_make_channel(lua_State* L, ev_loop_t* loop)
{
    lev_channel_t* self = lua_newuserdata(L, sizeof(lev_channel_t));
    memset(self, 0, sizeof(*self));

    self->loop = loop;
    self->is_closed = 0;
    ev_list_init(&self->data_queue);
    ev_list_init(&self->wait_queue);
    ev_list_init(&self->busy_queue);

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lev_channel_gc },
        { NULL,     NULL },
    };
    static const luaL_Reg s_method[] = {
        { "close",  _lev_channel_close },
        { "send",   _lev_channel_send },
        { "recv",   _lev_channel_recv },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, LEV_CHANNEL_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return self;
}

int lev_channel(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    _lev_make_channel(L, loop);

    return 1;
}
