#include "test.lua.h"

TEST_F(lua, timer)
{
    static const char* script =
"local loop = ev.loop()\n"
"local sleep_time = 10\n"
"local t1 = 0\n"
"local t2 = 0\n"
"loop:co(function()\n"
"    t1 = ev.hrtime()\n"
"    loop:sleep(sleep_time)\n"
"    t2 = ev.hrtime()\n"
"end)\n"
"loop:run()\n"
"assert(t2 - t1 >= sleep_time * 1000)\n";

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
