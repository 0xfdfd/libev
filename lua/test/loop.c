#include "test.lua.h"

TEST_F(lua, loop)
{
    static const char* script =
"local loop = ev.mkloop()\n"
"local ret1 = 0\n"
"loop:co(function(a, b)\n"
"    ret1 = a + b\n"
"end, 1, 2)\n"
"local ret2 = 0\n"
"loop:co(function(a, b)\n"
"    ret2 = a * b\n"
"end, 1, 2)\n"
"loop:run()\n"
"assert(ret1 == 3)\n"
"assert(ret2 == 2)\n"
;

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}

TEST_F(lua, loop_empty)
{
    static const char* script =
"local loop = ev.mkloop()\n"
"loop:run()\n";

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
