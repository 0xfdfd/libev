#include "sha256.h"
#include "function.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#   define strtok_r(str, delim, saveptr) strtok_s(str, delim, saveptr)
#endif

int am_sha256(lua_State* L)
{
    size_t data_sz = 0;
    const uint8_t* data = (const uint8_t*)luaL_checklstring(L, 1, &data_sz);

    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, data_sz);

    uint8_t ret[SHA256_BLOCK_SIZE];
    sha256_final(&ctx, ret);

    char buf[65]; size_t i;
    for (i = 0; i < sizeof(ret); i++)
    {
        snprintf(buf + 2 * i, sizeof(buf) - 2 * i, "%02x", ret[i]);
    }

    lua_pushstring(L, buf);
    return 1;
}

int am_readfile(lua_State* L)
{
    FILE* file;
    const char* path = luaL_checkstring(L, 1);

#if defined(_WIN32)
    if (fopen_s(&file, path, "rb") != 0)
#else
    if ((file = fopen(path, "rb")) == NULL)
#endif
    {
        return luaL_error(L, "open file `%s` failed.", path);
    }

    fseek(file, 0, SEEK_END);
    size_t file_sz = ftell(file);
    rewind(file);

    char* file_dat = malloc(file_sz);
    if (file_dat == NULL)
    {
        return luaL_error(L, "out of memory");
    }

    if (fread(file_dat, file_sz, 1, file) != 1)
    {
        free(file_dat);
        return luaL_error(L, "read file `%s` failed.", path);
    }

    lua_pushlstring(L, file_dat, file_sz);

    free(file_dat);
    fclose(file);

    return 1;
}

int am_writefile(lua_State* L)
{
    FILE* file;
    const char* path = luaL_checkstring(L, 1);

    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 2, &data_sz);

#if defined(_WIN32)
    if (fopen_s(&file, path, "wb") != 0)
#else
    if ((file = fopen(path, "wb")) == NULL)
#endif
    {
        return luaL_error(L, "open file `%s` failed.", path);
    }

    if (fwrite(data, data_sz, 1, file) != 1)
    {
        fclose(file);
        return luaL_error(L, "write file `%s` failed", path);
    }

    fclose(file);
    return 0;
}

int am_split_line(lua_State* L)
{
    size_t data_sz = 0;
    const char* data = luaL_checklstring(L, 1, &data_sz);

    // This will be the return value.
    lua_newtable(L);

    size_t pos;
    const char* start = data;

    for (pos = 0; pos < data_sz; pos++)
    {
        /*
         * There are 3 EOLs:
         * \r\n: CRLF (Windows)
         * \r: CR (Macintosh)
         * \n: LF (Unix)
         * Here we allow mixed line endings, just to parse aggressively.
         */
        if (data[pos] == '\r')
        {
            size_t len = &data[pos] - start;
            lua_pushlstring(L, start, len);
            lua_seti(L, -2, luaL_len(L, -2) + 1);

            if (pos < data_sz - 1 && data[pos + 1] == '\n')
            {
                start = &data[pos + 2];
                pos++;
            }
            else
            {
                start = &data[pos + 1];
            }
            continue;
        }
        else if (data[pos] == '\n')
        {
            size_t len = &data[pos] - start;
            lua_pushlstring(L, start, len);
            lua_seti(L, -2, luaL_len(L, -2) + 1);

            start = &data[pos + 1];
            continue;
        }
    }

    return 1;
}
