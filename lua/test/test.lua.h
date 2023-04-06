#ifndef __TEST_LUA_H__
#define __TEST_LUA_H__

#include "test.h"
#include "ev.lua.h"

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

void test_lua_set_test_integer(const char* key, int value);

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
