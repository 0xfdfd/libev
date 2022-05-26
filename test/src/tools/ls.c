#include "ls.h"
#include "ev.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#   include <windows.h>
#else
#   include <dirent.h>
#endif

typedef struct file_info
{
    ev_map_node_t   node;
#if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    char            name[];
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif
}file_info_t;

typedef struct ls_ctx
{
    ev_map_t    node_table;
} ls_ctx_t;

static int _on_cmp_file_info(const ev_map_node_t* key1,
    const ev_map_node_t* key2, void* arg)
{
    (void)arg;

    file_info_t* info1 = EV_CONTAINER_OF(key1, file_info_t, node);
    file_info_t* info2 = EV_CONTAINER_OF(key2, file_info_t, node);

    return strcmp(info1->name, info2->name);
}

static ls_ctx_t g_ls_ctx = {
    EV_MAP_INIT(_on_cmp_file_info, NULL),
};

#if defined(_WIN32)

static int _ls_read_dir(void)
{
    WIN32_FIND_DATAA finfo;

    HANDLE dir_handle = FindFirstFileA("*", &finfo);
    if (dir_handle == INVALID_HANDLE_VALUE)
    {
        return EXIT_FAILURE;
    }

    do
    {
        size_t name_len = strlen(finfo.cFileName) + 1;
        file_info_t* info = malloc(sizeof(file_info_t) + name_len);
        memcpy(info->name, finfo.cFileName, name_len);
        ev_map_insert(&g_ls_ctx.node_table, &info->node);
    } while (FindNextFileA(dir_handle, &finfo));

    FindClose(dir_handle);

    return EXIT_SUCCESS;
}

#else

static int _ls_read_dir(void)
{
    DIR* d = opendir(".");
    if (d == NULL)
    {
        return EXIT_SUCCESS;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        size_t name_len = strlen(dir->d_name) + 1;
        file_info_t* info = malloc(sizeof(file_info_t) + name_len);
        memcpy(info->name, dir->d_name, name_len);
        ev_map_insert(&g_ls_ctx.node_table, &info->node);
    }
    closedir(d);

    return 0;
}
#endif

static void _ls_show_files(void)
{
    ev_map_node_t* it = ev_map_begin(&g_ls_ctx.node_table);
    for (; it != NULL; it = ev_map_next(it))
    {
        file_info_t* info = EV_CONTAINER_OF(it, file_info_t, node);
        printf("%s\n", info->name);
    }
}

static void _ls_cleanup(void)
{
    ev_map_node_t* it;
    while ((it = ev_map_begin(&g_ls_ctx.node_table)) != NULL)
    {
        file_info_t* info = EV_CONTAINER_OF(it, file_info_t, node);
        ev_map_erase(&g_ls_ctx.node_table, it);
        free(info);
    }
}

static int tool_ls(int argc, char* argv[])
{
    (void)argc; (void)argv;

    int ret;

    if ((ret = _ls_read_dir()) != 0)
    {
        return ret;
    }

    _ls_show_files();
    _ls_cleanup();

    return 0;
}

test_tool_t test_tool_ls = {
    "ls", tool_ls,
    "List file names in current directory."
};