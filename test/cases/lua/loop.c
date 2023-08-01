#include "test.lua.h"

TEST_F(lua, DISABLED_loop)
{
    static const char* script =
"local loop = ev.loop()\n"
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

    TEST_CALL_LUA(script);
}

TEST_F(lua, DISABLED_loop_empty)
{
    static const char* script =
"local loop = ev.loop()\n"
"loop:run()\n";

    TEST_CALL_LUA(script);
}
