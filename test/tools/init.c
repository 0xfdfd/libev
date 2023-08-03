#include "echoserver.h"
#include "eolcheck.h"
#include "help.h"
#include "ls.h"
#include "pwd.h"
#include "init.h"
#include "test.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static test_tool_t* g_command_table[] = {
    &test_tool_echoserver,
    &test_tool_eolcheck,
    &test_tool_ls,
    &test_tool_pwd,
    &test_tool_help,
};

int tool_exec(int argc, char* argv[])
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(g_command_table); i++)
    {
        if (strcmp(argv[0], g_command_table[i]->cmd) != 0)
        {
            continue;
        }

        return g_command_table[i]->entrypoint(argc, argv);
    }

    fprintf(stderr, "%s: command not found\n", argv[0]);
    fflush(NULL);

    return -1;
}

void tool_foreach(int (*cb)(test_tool_t*, void*), void* arg)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(g_command_table); i++)
    {
        if (cb(g_command_table[i], arg))
        {
            return;
        }
    }
}
