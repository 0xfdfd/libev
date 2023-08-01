#include "test.lua.h"

TEST_F(lua, DISABLED_channel)
{
    static const char* script =
"local loop = ev.loop()\n"
"local chan = loop:channel()\n"

"local function sender()\n"
"    for i=1,10 do\n"
"        chan:send(i)\n"
"    end\n"
"    chan:close()\n"
"end\n"

"local function recver()\n"
"    for i=1,10 do\n"
"        local e, v = chan:recv()\n"
"        assert(e == nil)\n"
"        assert(v == i)\n"
"    end\n"
"    local e, v = chan:recv()\n"
"    assert(e ~= nil)\n"
"    assert(v == nil)\n"
"end\n"

"loop:co(recver)\n"
"loop:co(sender)\n"
"loop:run()\n";

    TEST_CALL_LUA(script);
}
