#include "timer.lua.h"

typedef struct lev_timer
{
    ev_timer_t      timer;
    ev_loop_t*      loop;
    lua_State*      L;
} lev_timer_t;

static void _lev_on_timer(ev_timer_t* timer)
{
    lev_timer_t* self = EV_CONTAINER_OF(timer, lev_timer_t, timer);
    lev_set_state(self->L, self->loop, 1);
}

static int _lev_on_sleep_resume(lua_State* L, int status, lua_KContext ctx)
{
    (void)L; (void)status; (void)ctx;
    return 0;
}

static int _lev_timer_gc(lua_State* L)
{
    lev_timer_t* timer = lua_touserdata(L, 1);

    ev_timer_exit(&timer->timer, NULL);
    ev_loop_run(timer->loop, EV_LOOP_MODE_NOWAIT);

    return 0;
}

int lev_sleep(lua_State* L)
{
    int ret;
    ev_loop_t* loop = lev_to_loop(L, 1);
    uint64_t timeout = luaL_checkinteger(L, 2);

    lev_timer_t* timer = lua_newuserdata(L, sizeof(lev_timer_t));
    if ((ret = ev_timer_init(loop, &timer->timer)) != 0)
    {
        return lev_error(L, loop, ret, NULL);
    }

    timer->L = L;
    timer->loop = loop;

    static const luaL_Reg s_meta[] = {
        { "__gc",   _lev_timer_gc },
        { NULL,     NULL },
    };
    if (luaL_newmetatable(L, "__ev_timer") != 0)
    {
        luaL_setfuncs(L, s_meta, 0);
    }
    lua_setmetatable(L, -2);

    if ((ret = ev_timer_start(&timer->timer, _lev_on_timer, timeout, 0)) != 0)
    {
        return lev_error(L, loop, ret, NULL);
    }

    lev_set_state(L, loop, 0);
    return lua_yieldk(L, 0, (lua_KContext)NULL, _lev_on_sleep_resume);
}
