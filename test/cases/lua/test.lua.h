#ifndef __TEST_LUA_H__
#define __TEST_LUA_H__

#include "test.h"

/**
 * @brief Call Lua script with arguments.
 *
 * The arguments list is stored into a global table named `arg`. The `arg`
 * table follow syntax:
 * 1. The first argument is at index 1, the second at index 2, and so on.
 * 2. If argument have syntax `--foo=bar`, the `bar` string can be accessed by `arg.foo`
 */
#define TEST_CALL_LUA(s, ...)   \
    do {\
        const char* args[] = { NULL, ##__VA_ARGS__, NULL };\
        ASSERT_EQ_INT(test_lua_dostring(g_test_lua.L, s, args), LUA_OK,\
            "%s", lua_tostring(g_test_lua.L, -1));\
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct test_lua
{
    /**
     * @brief Lua virtual machine.
     */
    lua_State* L;
} test_lua_t;

/**
 * @brief Global Lua runtime.
 * This runtime will be reset before each test.
 *
 * By default two global components is injected into this Lua VM:
 *
 * | Name | Feature                                   |
 * | ---- | ----------------------------------------- |
 * | ev   | libev's Lua interface                     |
 * | test | A global dict contains test specific data |
 *
 */
extern test_lua_t g_test_lua;

/**
 * @brief Setup Lua test environment.
 */
void test_lua_setup(void);

/**
 * @brief Teardown Lua test environment.
 */
void test_lua_teardown(void);

/**
 * @brief Set string value of \p value into test object.
 * @param[in] key   Key.
 * @param[in] value String value.
 */
void test_lua_set_test_string(const char* key, const char* value);

/**
 * @brief Set integer value of \p value into test object.
 * @param[in] key   Key.
 * @param[in] value String value.
 */
void test_lua_set_test_integer(const char* key, int value);

/**
 * @brief Like luaL_dostring(), but with traceback.
 */
int test_lua_dostring(lua_State* L, const char* s, const char** args);

/*
 * Common Lua test environment setup and teardown progress.
 * This is enabled by default. To disable it, define `EV_TEST_NO_LUA_SETUP`
 * before include this header.
 */
#if !defined(EV_TEST_NO_LUA_SETUP)

TEST_FIXTURE_SETUP(lua)
{
    test_lua_setup();
}

TEST_FIXTURE_TEARDOWN(lua)
{
    test_lua_teardown();
}

#endif

#ifdef __cplusplus
}
#endif
#endif
