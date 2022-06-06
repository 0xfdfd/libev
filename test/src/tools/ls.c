#include "ls.h"
#include "test.h"
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
    ev_loop_t       loop;
    ev_threadpool_t pool;
    ev_os_thread_t  thread_storage[1];
    ev_fs_req_t     fs_req;
    ev_map_t        node_table;
    const char*     ls_path;
} ls_ctx_t;

static int _on_cmp_file_info(const ev_map_node_t* key1,
    const ev_map_node_t* key2, void* arg)
{
    (void)arg;

    file_info_t* info1 = EV_CONTAINER_OF(key1, file_info_t, node);
    file_info_t* info2 = EV_CONTAINER_OF(key2, file_info_t, node);

    return strcmp(info1->name, info2->name);
}

static const char* s_ls_help =
"Usage: ls [OPTION]... [FILE]...\n"
"  --help\n"
"      display this help and exit\n"
;

static ls_ctx_t g_ls_ctx = {
    EV_LOOP_INVALID,
    EV_THREADPOOL_INVALID,
    { EV_OS_THREAD_INVALID },
    EV_FS_REQ_INVALID,
    EV_MAP_INIT(_on_cmp_file_info, NULL),
    "./",
};

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
        ev_map_erase(&g_ls_ctx.node_table, &info->node);
        mmc_free(info);
    }

    ev_loop_exit(&g_ls_ctx.loop);
    ev_threadpool_exit(&g_ls_ctx.pool);
}

static void _ls_on_readdir(ev_fs_req_t* req)
{
    ev_dirent_t* it = ev_fs_get_first_dirent(req);
    for (; it != NULL; it = ev_fs_get_next_dirent(it))
    {
        size_t name_len = strlen(it->name) + 1;
        size_t malloc_size = sizeof(file_info_t) + name_len;
        file_info_t* info = mmc_malloc(malloc_size);
        memcpy(info->name, it->name, name_len);
        ev_map_insert(&g_ls_ctx.node_table, &info->node);
    }

    ev_fs_req_cleanup(req);
}

static int _parser_options(int argc, char* argv[], int* need_exit)
{
    *need_exit = 0;

    if (argc <= 1)
    {
        return 0;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        goto show_help_and_exit;
    }

    g_ls_ctx.ls_path = argv[1];

    return 0;

show_help_and_exit:
    printf("%s", s_ls_help);
    *need_exit = 1;

    return 0;
}

static int tool_ls(int argc, char* argv[])
{
    int ret, flag_need_exit;
    if ((ret = _parser_options(argc, argv, &flag_need_exit)) != 0)
    {
        return ret;
    }
    if (flag_need_exit)
    {
        return 0;
    }

    if ((ret = ev_loop_init(&g_ls_ctx.loop)) != EV_SUCCESS)
    {
        fprintf(stderr, "%s\n", ev_strerror(ret));
        return EXIT_FAILURE;
    }

    ret = ev_threadpool_init(&g_ls_ctx.pool, NULL, g_ls_ctx.thread_storage,
        ARRAY_SIZE(g_ls_ctx.thread_storage));
    if (ret != EV_SUCCESS)
    {
        ev_loop_exit(&g_ls_ctx.loop);
        fprintf(stderr, "%s\n", ev_strerror(ret));
        return EXIT_FAILURE;
    }

    ev_loop_link_threadpool(&g_ls_ctx.loop, &g_ls_ctx.pool);

    ret = ev_fs_readdir(&g_ls_ctx.loop, &g_ls_ctx.fs_req, g_ls_ctx.ls_path, _ls_on_readdir);
    if (ret != EV_SUCCESS)
    {
        fprintf(stderr, "%s\n", ev_strerror(ret));
        ret = EXIT_FAILURE;
        goto finish;
    }

    ret = ev_loop_run(&g_ls_ctx.loop, EV_LOOP_MODE_DEFAULT);
    if (ret != EV_SUCCESS)
    {
        fprintf(stderr, "%s\n", ev_strerror(ret));
        ret = EXIT_FAILURE;
        goto finish;
    }

    _ls_show_files();

finish:
    _ls_cleanup();
    return ret;
}

test_tool_t test_tool_ls = {
    "ls", tool_ls,
    "List file names in current directory."
};