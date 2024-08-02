#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "__init__.h"
#include "utils/memcheck.h"
#include "utils/file.h"
#include "utils/str.h"

typedef struct tabcheck_cfg
{
    const char* path;
} tabcheck_cfg_t;

static int _tabcheck_get_config(tabcheck_cfg_t* cfg, int argc, char* argv[])
{
    int i;
    const char* opt;
    size_t opt_sz;

    cfg->path = NULL;
    for (i = 0; i < argc; i++)
    {
        opt = "--file="; opt_sz = strlen(opt);
        if (strncmp(argv[i], opt, opt_sz) == 0)
        {
            cfg->path = argv[i] + opt_sz;
            continue;
        }
    }

    if (cfg->path == NULL)
    {
        fprintf(stderr, "missing argument `--file`.\n");
        return EXIT_FAILURE;
    }

    return 0;
}

static int _tabcheck_check(const char* name, char* data)
{
    char* saveptr = NULL;
    char* line = NULL;

    unsigned cnt_line = 1;
    for (; (line = _eolcheck_strtok(data, "\r\n", &saveptr)) != NULL; cnt_line++)
    {
        if (strchr(line, '\t') != NULL)
        {
            size_t line_sz = strlen(line);
            fprintf(stderr, "TAB found on line %u in file `%s`.\n", cnt_line, name);
            mmc_dump_hex(line, line_sz, 16);
            return EXIT_FAILURE;
        }
    }

    return 0;
}

static int tool_tabcheck(int argc, char* argv[])
{
    int ret;

    tabcheck_cfg_t cfg;
    if ((ret = _tabcheck_get_config(&cfg, argc, argv)) != 0)
    {
        return ret;
    }

    char* content = NULL;
    if ((ret = (int)test_read_file(cfg.path, &content)) < 0)
    {
        return ret;
    }

    ret = _tabcheck_check(cfg.path, content);
    ev_free(content);

    return ret;
}

const test_tool_t test_tool_tabcheck = {
"tabcheck", tool_tabcheck,
"Check if a file contains table.\n"
"  --file=[PATH]\n"
"    Path to check.\n"
};
