#define EV_TEST_NO_LUA_SETUP
#include "test.lua.h"

#define TEST_LUA_TMPFILE_PATH   "353298c2-1c3a-46ab-9cdd-265e5bf04da5"

static void _test_lua_remove_tmpfile(void)
{
    ev_fs_remove_sync(TEST_LUA_TMPFILE_PATH, 1);
}

TEST_FIXTURE_SETUP(lua)
{
    test_lua_setup();
    _test_lua_remove_tmpfile();
}

TEST_FIXTURE_TEARDOWN(lua)
{
    test_lua_teardown();
    _test_lua_remove_tmpfile();
}

TEST_F(lua, process_pipe)
{
    static const char* script =
"local loop = ev.loop()\n"
"local self_path = ev.exepath()\n"
"local out_pipe = loop:pipe()\n"
"loop:co(function()\n"
"    local opt = { file = self_path, stdout = out_pipe, argv = { self_path, \"--help\" } }\n"
"    local _, process = loop:process(opt)\n"
"    assert(process ~= nil)\n"
"    local _, data = out_pipe:recv()\n"
"    assert(data ~= nil)\n"
"end)\n"
"loop:run()\n";

    ASSERT_EQ_INT(test_lua_dostring(g_test_lua.L, script, NULL), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}

TEST_F(lua, process_file)
{
    static const char* script =
"local loop = ev.loop()\n"
"loop:co(function()\n"
"    local _, out_file = loop:fs_file(test.path, ev.EV_FS_O_CREAT | ev.EV_FS_O_WRONLY)\n"
"    assert(out_file ~= nil)\n"
"    local err, process = loop:process({\n"
"        file = ev.exepath(),\n"
"        stdout = out_file,\n"
"        argv = { ev.exepath(), \"--help\" },\n"
"    })\n"
"    assert(err == nil)\n"
"    assert(process ~= nil)\n"
"    process:waitpid()\n"
"end)\n"
"loop:run()\n";

    test_lua_set_test_string("path", TEST_LUA_TMPFILE_PATH);

    ASSERT_EQ_INT(test_lua_dostring(g_test_lua.L, script, NULL), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}

TEST_F(lua, DISABLED_process_tcp)
{
    static const char* script =
"local loop = ev.loop()\n"
"local srv = loop:tcp()\n"
"srv:listen(ev.ip_addr(test.ip, test.port))\n"
"local pip_stdin = loop:pipe()\n"
"local pip_stdout = loop:pipe()\n"
"loop:co(function()\n"
"    local _, cli = srv:accept()\n"
"    local _, proc = loop:process({\n"
"        file = ev.exepath(),\n"
"        argv = { ev.exepath(), \"--\", \"echoserver\" },\n"
"        stdin = pip_stdin,\n"
"        stdout = pip_stdout,\n"
"    })\n"
"    assert(proc ~= nil)\n"
"    loop:co(function()\n"
"        while true do\n"
"            local _, data = cli:recv()\n"
"            pip_stdin:send(data)\n"
"        end\n"
"    end)\n"
"    loop:co(function()\n"
"        while true do\n"
"            local _, data = pip_stdout:recv()\n"
"            cli:send(data)\n"
"        end\n"
"    end)\n"
"end)\n"
"loop:run()\n";

    test_lua_set_test_string("ip", "0.0.0.0");
    test_lua_set_test_integer("port", 5000);

    ASSERT_EQ_INT(test_lua_dostring(g_test_lua.L, script, NULL), LUA_OK,
        "%s", lua_tostring(g_test_lua.L, -1));
}
