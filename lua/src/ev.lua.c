#include "channel.lua.h"
#include "fs.lua.h"
#include "misc.lua.h"
#include "pipe.lua.h"
#include "process.lua.h"
#include "tcp.lua.h"
#include "timer.lua.h"
#include "udp.lua.h"
#include <assert.h>

/**
 * @brief Map of ev public interface.
 */
#define LEV_API_MAP(xx)                 \
    xx("exepath",       lev_exepath )   \
    xx("getcwd",        lev_getcwd)     \
    xx("hrtime",        lev_hrtime)     \
    xx("ip_addr",       lev_ip_addr)    \
    xx("ip_name",       lev_ip_name)    \
    xx("strerror",      lev_strerror)

/**
 * @brief Map of loop public interface.
 */
#define LEV_LOOP_API_MAP(xx)            \
    xx("channel",       lev_channel)    \
    xx("fs_file",       lev_fs_file)    \
    xx("fs_remove",     lev_fs_remove)  \
    xx("pipe",          lev_pipe)       \
    xx("process",       lev_process)    \
    xx("sleep",         lev_sleep)      \
    xx("tcp",           lev_tcp)        \
    xx("udp",           lev_udp)

/**
 * @brief Map of public flags.
 */
#define LEV_OPT_MAP(xx)                 \
    LEV_FILE_FLAG_MAP(xx)               \
    LEV_UDP_FLAG_MAP(xx)

/**
 * @brief Loop type name in Lua.
 */
#define LEV_LOOP_NAME                   "__ev_loop"

typedef struct lev_loop_s
{
    ev_loop_t       loop;

    ev_map_t        record;
    ev_list_t       wait_queue;
    ev_list_t       busy_queue;
} lev_loop_t;

typedef struct lev_coroutine
{
    ev_map_node_t   r_node;
    ev_list_node_t  q_node;     /**< Node for #lev_loop_t::busy_queue or #lev_loop_t::wait_queue. */

    lev_loop_t*     belong;     /**< Belong loop. */
    lua_State*      thread;     /**< Coroutine handle. */
    int             ref_thread; /**< Coroutine reference. */
    int             nresult;    /**< The number of parameter / results. */

    int             ref_self;   /**< Reference to self. */
    int             is_busy;    /**< If in busy_queue. */
} lev_coroutine_t;

static int _lev_on_cmp_co(const ev_map_node_t* key1, const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    lev_coroutine_t* co1 = EV_CONTAINER_OF(key1, lev_coroutine_t, r_node);
    lev_coroutine_t* co2 = EV_CONTAINER_OF(key2, lev_coroutine_t, r_node);
    if (co1->thread == co2->thread)
    {
        return 0;
    }
    return co1->thread < co2->thread ? -1 : 1;
}

static void _lev_co_unlink(lev_coroutine_t* co)
{
    if (co->belong == NULL)
    {
        return;
    }

    if (co->is_busy)
    {
        ev_list_erase(&co->belong->busy_queue, &co->q_node);
    }
    else
    {
        ev_list_erase(&co->belong->wait_queue, &co->q_node);
    }
    ev_map_erase(&co->belong->record, &co->r_node);

    co->belong = NULL;
}

static void _lev_release_co(lua_State* L, lev_coroutine_t* co)
{
    /* Remove from scheduler. */
    _lev_co_unlink(co);

    /* No let GC handle left things. */
    luaL_unref(L, LUA_REGISTRYINDEX, co->ref_self);
    co->ref_self = LUA_NOREF;
}

static int _lev_loop_gc(lua_State* L)
{
    int ret;
    ev_list_node_t* it;
    lev_loop_t* loop = lua_touserdata(L, 1);

    /* Remove reference to all coroutine. */
    while ((it = ev_list_begin(&loop->busy_queue)) != NULL)
    {
        lev_coroutine_t* co = EV_CONTAINER_OF(it, lev_coroutine_t, q_node);
        _lev_release_co(L, co);
    }
    while ((it = ev_list_begin(&loop->wait_queue)) != NULL)
    {
        lev_coroutine_t* co = EV_CONTAINER_OF(it, lev_coroutine_t, q_node);
        _lev_release_co(L, co);
    }

    /* Exit loop. */
    if ((ret = ev_loop_exit(&loop->loop)) != 0)
    {
        return lev_error(L, &loop->loop, ret, NULL);
    }

    return 0;
}

static int _lev_co_gc(lua_State* L)
{
    lev_coroutine_t* co = lua_touserdata(L, 1);

    _lev_co_unlink(co);

    if (co->ref_self != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, co->ref_self);
        co->ref_self = LUA_NOREF;
    }

    if (co->ref_thread != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, co->ref_thread);
        co->ref_thread = LUA_NOREF;
    }
    co->thread = NULL;

    return 0;
}

static void _lev_copy_stack(lua_State* L, lev_coroutine_t* co, int beg, int end)
{
    int i;
    for (i = beg; i <= end; i++)
    {
        lua_pushvalue(L, i);
    }
    lua_xmove(L, co->thread, end - beg + 1);

    co->nresult = end - beg;
}

static void _lev_co_set_state(lev_coroutine_t* co, int is_busy)
{
    if (co->is_busy == is_busy)
    {
        return;
    }

    if (co->is_busy)
    {
        ev_list_erase(&co->belong->busy_queue, &co->q_node);
        ev_list_push_back(&co->belong->wait_queue, &co->q_node);
    }
    else
    {
        ev_list_erase(&co->belong->wait_queue, &co->q_node);
        ev_list_push_back(&co->belong->busy_queue, &co->q_node);
    }
    co->is_busy = is_busy;
}

static int _lev_loop_onpass(lua_State* L, lev_loop_t* loop)
{
    ev_list_node_t* it;
    while ((it = ev_list_begin(&loop->busy_queue)) != NULL)
    {
        lev_coroutine_t* co = EV_CONTAINER_OF(it, lev_coroutine_t, q_node);

        int ret = lua_resume(co->thread, L, co->nresult, &co->nresult);

        /* Coroutine yield */
        if (ret == LUA_YIELD)
        {
            /* Anything received treat as busy coroutine */
            if (co->nresult != 0)
            {
                lua_pop(co->thread, co->nresult);
                co->nresult = 0;
            }

            continue;
        }

        /* Error affect main thread */
        if (ret != LUA_OK)
        {
            lua_xmove(co->thread, L, 1);
            return lua_error(L);
        }

        /* Finish execution. */
        _lev_release_co(L, co);
    }

    return 0;
}

static int _lev_loop_run_once(lua_State* L, lev_loop_t* loop, int mode)
{
    _lev_loop_onpass(L, loop);

    if (ev_list_size(&loop->wait_queue) != 0)
    {
        ev_loop_run(&loop->loop, mode);
    }

    _lev_loop_onpass(L, loop);

    return 0;
}

static int _lev_loop_run_default(lua_State* L, lev_loop_t* loop)
{
    while (1)
    {
        _lev_loop_run_once(L, loop, EV_LOOP_MODE_ONCE);

        if (ev_list_size(&loop->wait_queue) == 0 && ev_list_size(&loop->busy_queue) == 0)
        {
            break;
        }
    }

    return 0;
}

static void _lev_set_options(lua_State* L)
{
#define IMPORT_OPTIONS(x)   \
    lua_pushinteger(L, x);\
    lua_setfield(L, -2, #x);

    LEV_OPT_MAP(IMPORT_OPTIONS);

#undef IMPORT_OPTIONS

    lua_pushlightuserdata(L, NULL);
    lua_setfield(L, -2, "NULL");
}

static char* _lev_strerr(const char* fmt, va_list ap)
{
    int cnt;
    if (fmt == NULL)
    {
        return NULL;
    }

    {
        va_list ap_bak;
        va_copy(ap_bak, ap);
        cnt = vsnprintf(NULL, 0, fmt, ap_bak);
        va_end(ap_bak);
    }

    char* buf = ev_malloc(cnt + 1);
    snprintf(buf, cnt + 1, fmt, ap);
    return buf;
}

static void _lev_dump_error_as_json_str(luaL_Buffer* b, const char* umsg)
{
    size_t i;
    char c;
    char buf[8];

    if (umsg == NULL)
    {
        luaL_addstring(b, "null");
        return;
    }

    luaL_addchar(b, '\"');

    for (i = 0; (c = umsg[i]) != '\0'; i++)
    {
        if ((c > 31) && (c != '\"') && (c != '\\'))
        {
            luaL_addchar(b, c);
        }
        else
        {
            luaL_addchar(b, '\\');
            switch (c)
            {
            case '\\': luaL_addchar(b, '\\'); break;
            case '\"': luaL_addchar(b, '\"'); break;
            case '\b': luaL_addchar(b, 'b'); break;
            case '\f': luaL_addchar(b, 'f'); break;
            case '\n': luaL_addchar(b, 'n'); break;
            case '\r': luaL_addchar(b, 'r'); break;
            case '\t': luaL_addchar(b, 't'); break;
            default:
                snprintf(buf, sizeof(buf), "u%04x", (unsigned int)c);
                luaL_addstring(b, buf);
                break;
            }
        }
    }

    luaL_addchar(b, '\"');
}

static const char* _lev_push_error_object(const char* file, int line, lua_State* L, ev_loop_t* loop, int errcode, const char* msg)
{
    char buffer[4096];
    luaL_Buffer buf;
    luaL_buffinit(L, &buf);

    snprintf(buffer, sizeof(buffer),
        "{"
        "\"file\":\"%s\","
        "\"line\":%d,"
        "\"loop\":\"%p\","
        "\"errnum\":%d,"
        "\"errmsg\":\"%s\","
        "\"extmsg\":"
        , file, line, loop, errcode, ev_strerror(errcode));
    luaL_addstring(&buf, buffer);

    _lev_dump_error_as_json_str(&buf, msg);
    luaL_addstring(&buf, "}");

    luaL_pushresult(&buf);

    return lua_tostring(L, -1);
}

static lev_coroutine_t* _lev_find_coroutine(lua_State* L, ev_loop_t* loop)
{
    lev_loop_t* impl = EV_CONTAINER_OF(loop, lev_loop_t, loop);

    lev_coroutine_t tmp; tmp.thread = L;
    ev_map_node_t* it = ev_map_find(&impl->record, &tmp.r_node);
    if (it == NULL)
    {
        return NULL;
    }

    lev_coroutine_t* co = EV_CONTAINER_OF(it, lev_coroutine_t, r_node);
    return co;
}

/**
 * @brief Same as #lev_make_loop() but always return 1.
 * @param[in] L     Lua Stack.
 * @return          Always 1.
 */
static int _lev_mkloop(lua_State* L)
{
    lev_make_loop(L);
    return 1;
}

int luaopen_ev(lua_State* L)
{
#if LUA_VERSION_NUM >= 502
    luaL_checkversion(L);
#endif

    static const luaL_Reg s_ev_api_map[] = {
        { "mkloop",     _lev_mkloop },
#define EXPAND_EV_LUA_API_MAP(n, f) { n, f },
        LEV_API_MAP(EXPAND_EV_LUA_API_MAP)
#undef EXPAND_EV_LUA_API_MAP
        { NULL,         NULL },
    };
    luaL_newlib(L, s_ev_api_map);

    _lev_set_options(L);

    return 1;
}

static int _lev_loop_co(lua_State* L)
{
    lev_loop_t* loop = luaL_checkudata(L, 1, LEV_LOOP_NAME);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    int sp = lua_gettop(L);

    lev_coroutine_t* co = lua_newuserdata(L, sizeof(lev_coroutine_t));
    memset(co, 0, sizeof(*co));

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lev_co_gc },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, "__ev_coroutine") != 0)
    {
        luaL_setfuncs(L, s_meta, 0);
    }
    lua_setmetatable(L, -2);

    {
        co->thread = lua_newthread(L);
        co->ref_thread = luaL_ref(L, LUA_REGISTRYINDEX);
        co->belong = loop;
    }

    _lev_copy_stack(L, co, 2, sp);

    co->ref_self = luaL_ref(L, LUA_REGISTRYINDEX);

    ev_list_push_back(&loop->busy_queue, &co->q_node);
    ev_map_insert(&loop->record, &co->r_node);
    co->is_busy = 1;

    return 0;
}

static int _lev_loop_run(lua_State* L)
{
    lev_loop_t* loop = luaL_checkudata(L, 1, LEV_LOOP_NAME);

    /* Get loop mode. */
    int mode = EV_LOOP_MODE_DEFAULT;
    if (lua_type(L, 2) == LUA_TNUMBER)
    {
        mode = (int)lua_tonumber(L, 2);
    }

    switch (mode)
    {
    case EV_LOOP_MODE_ONCE:
        return _lev_loop_run_once(L, loop, EV_LOOP_MODE_ONCE);

    case EV_LOOP_MODE_NOWAIT:
        return _lev_loop_run_once(L, loop, EV_LOOP_MODE_NOWAIT);

    default:
        break;
    }

    return _lev_loop_run_default(L, loop);
}

static int _lev_error(lua_State* L)
{
    ev_loop_t* loop = lev_to_loop(L, 1);
    int errcode = (int)luaL_checkinteger(L, 2);
    const char* msg = NULL;
    if (lua_type(L, 3) == LUA_TSTRING)
    {
        msg = lua_tostring(L, 3);
    }

    _lev_push_error_object(NULL, 0, L, loop, errcode, msg);
    return lua_error(L);
}

ev_loop_t* lev_make_loop(lua_State* L)
{
    lev_loop_t* loop = lua_newuserdata(L, sizeof(lev_loop_t));

    memset(loop, 0, sizeof(*loop));
    ev_loop_init(&loop->loop);
    ev_map_init(&loop->record, _lev_on_cmp_co, NULL);

    static const luaL_Reg s_meta[] = {
        { "__gc",       _lev_loop_gc },
        { NULL,         NULL },
    };
    static const luaL_Reg s_method[] = {
        /* Control */
        { "co",         _lev_loop_co },
        { "run",        _lev_loop_run },
        { "error",      _lev_error },
        /* Component */
#define EXPAND_EV_LOOP_API_MAP(n, f) { n, f },
        LEV_LOOP_API_MAP(EXPAND_EV_LOOP_API_MAP)
#undef EXPAND_EV_LOOP_API_MAP
        /* End */
        { NULL,         NULL },
    };
    if (luaL_newmetatable(L, LEV_LOOP_NAME) != 0)
    {
        luaL_setfuncs(L, s_meta, 0);

        /* metatable.__index = s_method */
        luaL_newlib(L, s_method);
        lua_setfield(L, -2, "__index");
    }
    lua_setmetatable(L, -2);

    return &loop->loop;
}

ev_loop_t* lev_to_loop(struct lua_State* L, int idx)
{
    lev_loop_t* loop = luaL_checkudata(L, idx, LEV_LOOP_NAME);
    return &loop->loop;
}

int lev_set_state(lua_State* L, ev_loop_t* loop, int busy)
{
    lev_coroutine_t* co = _lev_find_coroutine(L, loop);
    if (co == NULL)
    {
        return lev_error(L, loop, EV_ENOENT, "coroutine %p not found", L);
    }

    _lev_co_set_state(co, busy);
    return 0;
}

int lev_is_yieldable(lua_State* L, ev_loop_t* loop)
{
    lev_coroutine_t* co = _lev_find_coroutine(L, loop);
    if (co == NULL)
    {
        return 0;
    }

    return lua_isyieldable(L);
}

int lev_error_ex(const char* file, int line, lua_State* L, ev_loop_t* loop, int errcode, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char* umsg = _lev_strerr(fmt, ap);
    va_end(ap);

    _lev_push_error_object(file, line, L, loop, errcode, umsg);

    ev_free(umsg);

    return lua_error(L);
}
