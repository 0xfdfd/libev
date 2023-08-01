#include "test.lua.h"

TEST_F(lua, DISABLED_strerror)
{
    static const char* script =
"local str = ev.strerror(test.errcode)\n"
"assert(str == test.errstr)\n";

    int errcode = EV_ENOMEM;

    lua_newtable(g_test_lua.L);
    lua_pushinteger(g_test_lua.L, errcode);
    lua_setfield(g_test_lua.L, -2, "errcode");
    lua_pushstring(g_test_lua.L, ev_strerror(errcode));
    lua_setfield(g_test_lua.L, -2, "errstr");
    lua_setglobal(g_test_lua.L, "test");

    TEST_CALL_LUA(script);
}
