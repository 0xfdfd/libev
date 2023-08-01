#include "test.lua.h"

TEST_F(lua, DISABLED_udp)
{
    static const char* script =
"local loop = ev.loop()\n"
"local p = loop:promise()\n"
"loop:co(function()\n"
"    local sock = loop:udp()\n"
"    local err = sock:bind(ev.ip_addr(arg.ip, 0))\n"
"    assert(err == nil)\n"
"    local addr = sock:getsockname()\n"
"    p:set_value(ev.ip_name(addr))\n"
"    local _, data, addr = sock:recvfrom()\n"
"    assert(data == arg.data)\n"
"    err = sock:send(data, addr)\n"
"    assert(err == nil)\n"
"end)\n"
"loop:co(function()\n"
"    local sock = loop:udp()\n"
"    local _, port = p:get_value()\n"
"    local err = sock:connect(ev.ip_addr(arg.ip, port))\n"
"    assert(err == nil)\n"
"    err = sock:send(arg.data)\n"
"    assert(err == nil)\n"
"    local _, data = sock:recv()\n"
"    assert(data == arg.data)\n"
"end)\n"
"loop:run()\n";

    TEST_CALL_LUA(script, "--ip=127.0.0.1", "--data=a test data of hello world");
}
