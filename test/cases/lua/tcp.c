#include "test.lua.h"

TEST_F(lua, DISABLED_tcp)
{
    static const char* script =
"local loop = ev.loop()\n"
"local promise = loop:promise()\n"
"loop:co(function()\n"
"    local sock = loop:tcp()\n"
"    local err = sock:listen(ev.ip_addr(arg[1], 0))\n"
"    assert(err == nil)\n"
"    promise:set_value(ev.ip_name(sock:sockname()))\n"
"    local _, client = sock:accept()\n"
"    local _, data = client:recv()\n"
"    assert(data == arg[2])\n"
"    client:send(arg[2])\n"
"end)\n"
"loop:co(function()\n"
"    local sock = loop:tcp()\n"
"    local _, port = promise:get_value()\n"
"    local err = sock:connect(ev.ip_addr(arg[1], port))\n"
"    assert(err == nil)\n"
"    err = sock:send(arg[2])\n"
"    assert(err == nil)\n"
"    local _, data = sock:recv()\n"
"    assert(data == arg[2])\n"
"end)\n"
"loop:run()\n";

    TEST_CALL_LUA(script, "127.0.0.1", "hello world");
}
