#include "test.lua.h"

TEST_F(lua, promise)
{
    static const char* script =
"local loop = ev.loop()\n"
"local promise = loop:promise()\n"
"loop:co(function()\n"
"    local v = promise:get_value()\n"
"    assert(v == arg[1])\n"
"end)\n"
"loop:co(function()\n"
"    promise:set_value(arg[1])\n"
"end)\n"
"loop:run()\n";

    TEST_CALL_LUA(script, "a7366500-985c-45c9-9ea8-8775a9cf745b");
}
