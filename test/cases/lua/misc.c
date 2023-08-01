#include "test.lua.h"

TEST_F(lua, DISABLED_arg_pack)
{
    static const char* script =
"local args = ev.arg_pack(1, 2, 3)\n"
"local v1,v2,v3,v4 = ev.arg_unpack(args)\n"
"assert(v1 == 1)\n"
"assert(v2 == 2)\n"
"assert(v3 == 3)\n"
"assert(v4 == nil)\n";

    TEST_CALL_LUA(script);
}
