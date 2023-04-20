#include "test.lua.h"

TEST_F(lua, arg_pack)
{
    static const char* script =
"local args = ev.arg_pack(1, 2, 3)\n"
"local v1,v2,v3,v4 = ev.arg_unpack(args)\n"
"assert(v1 == 1)\n"
"assert(v2 == 2)\n"
"assert(v3 == 3)\n"
"assert(v4 == nil)\n";

    ASSERT_EQ_INT(luaL_dostring(g_test_lua.L, script), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
