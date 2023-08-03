#ifndef __AMALGAMATE_FUNCTION_H__
#define __AMALGAMATE_FUNCTION_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int am_sha256(lua_State* L);
int am_readfile(lua_State* L);
int am_writefile(lua_State* L);
int am_split_line(lua_State* L);

#ifdef __cplusplus
}
#endif
#endif
