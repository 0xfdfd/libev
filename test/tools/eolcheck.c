#include "eolcheck.h"
#include "utils/memcheck.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct eolcheck_cfg
{
    const char* path;
    const char* eol;
} eolcheck_cfg_t;

typedef struct eolcheck_file
{
    uint8_t* data;
    size_t data_sz;
} eolcheck_file_t;

#define EOLCHECK_FILE_INIT  { NULL, 0 }

static int _eolcheck_get_config(eolcheck_cfg_t* cfg, int argc, char* argv[])
{
    int i;
    const char* opt;

    cfg->path = NULL;
    cfg->eol = NULL;

    for (i = 0; i < argc; i++)
    {
        opt = "--file=";
        if (strncmp(argv[i], opt, strlen(opt)) == 0)
        {
            cfg->path = argv[i] + strlen(opt);
            continue;
        }

        opt = "--eol=";
        if (strncmp(argv[i], opt, strlen(opt)) == 0)
        {
            cfg->eol = argv[i] + strlen(opt);
            continue;
        }
    }

    if (cfg->path == NULL)
    {
        fprintf(stderr, "missing argument to `--file`.\n");
        return EXIT_FAILURE;
    }

    if (cfg->eol == NULL)
    {
        fprintf(stderr, "missing argument to `--eol`.\n");
        return EXIT_FAILURE;
    }

    if (strcmp(cfg->eol, "CR") != 0 && strcmp(cfg->eol, "LF") != 0 && strcmp(cfg->eol, "CRLF") != 0)
    {
        fprintf(stderr, "unknown option to `--eol`.\n");
        return EXIT_FAILURE;
    }

    return 0;
}

static int _eolcheck_read_file(eolcheck_file_t* dst, const char* path)
{
    FILE* file;

#if defined(_WIN32)
    if (fopen_s(&file, path, "rb") != 0)
#else
    if ((file = fopen(path, "rb")) == NULL)
#endif
    {
        fprintf(stderr, "failed to open `%s`.\n", path);
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    dst->data_sz = ftell(file);
    rewind(file);

    if ((dst->data = mmc_malloc(dst->data_sz + 1)) == NULL)
    {
        fprintf(stderr, "out of memory.\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    if (fread(dst->data, dst->data_sz, 1, file) != 1)
    {
        fprintf(stderr, "read file `%s` failed.\n", path);
        mmc_free(dst->data);
        dst->data = NULL;
        dst->data_sz = 0;
        return EXIT_FAILURE;
    }
    dst->data[dst->data_sz] = '\0';

    fclose(file);

    return 0;
}

static int _eolcheck_copy_file(eolcheck_file_t* dst, eolcheck_file_t* src)
{
    dst->data_sz = src->data_sz;
    if ((dst->data = mmc_malloc(dst->data_sz + 1)) == NULL)
    {
        fprintf(stderr, "out of memory.\n");
        return EXIT_FAILURE;
    }

    memcpy(dst->data, src->data, src->data_sz);
    dst->data[dst->data_sz] = '\0';

    return 0;
}

static void _eolcheck_release_file(eolcheck_file_t* file)
{
    if (file->data != NULL)
    {
        mmc_free(file->data);
        file->data = NULL;
    }
    file->data_sz = 0;
}

static int _eolcheck_is_match(char c, const char* delim)
{
    for (; *delim != '\0'; delim++)
    {
        if (*delim == c)
        {
            return 1;
        }
    }

    return 0;
}

static char* _eolcheck_strtok(char* str, const char* delim, char** saveptr)
{
    if (*saveptr == NULL)
    {
        *saveptr = str;
    }

    char* pos_start = *saveptr;

    for (; **saveptr != '\0'; *saveptr = *saveptr + 1)
    {
        if (_eolcheck_is_match(**saveptr, delim))
        {
            **saveptr = '\0';
            *saveptr = *saveptr + 1;
            return pos_start;
        }
    }

    return NULL;
}

static int _eolcheck_check_ending(eolcheck_file_t* file, eolcheck_cfg_t* cfg)
{
    int ret = 0;
    eolcheck_file_t cpy = EOLCHECK_FILE_INIT;
    _eolcheck_copy_file(&cpy, file);

    size_t cnt_line = 1;
    char* line = NULL;

    char* saveptr = NULL;
    for (; (line = _eolcheck_strtok((char*)cpy.data, "\r\n", &saveptr)) != NULL; cnt_line++)
    {
        size_t line_sz = strlen(line);
        size_t line_offset = line - (char*)cpy.data;
        size_t eol_offset = line_offset + line_sz;

        switch (file->data[eol_offset])
        {
        case '\r':
            if (eol_offset < file->data_sz && file->data[eol_offset + 1] == '\n')
            {
                saveptr++;
                if (strcmp(cfg->eol, "CRLF") != 0)
                {
                    fprintf(stderr, "CRLF found on line %u in file `%s`.\n", (unsigned)cnt_line, cfg->path);
                    mmc_dump_hex(file->data + line_offset, line_sz + 2, 16);
                    ret = EXIT_FAILURE;
                    goto fin;
                }
            }
            else
            {
                if (strcmp(cfg->eol, "CR") != 0)
                {
                    fprintf(stderr, "CR found on line %u in file `%s`.\n", (unsigned)cnt_line, cfg->path);
                    mmc_dump_hex(file->data + line_offset, line_sz + 1, 16);
                    ret = EXIT_FAILURE;
                    goto fin;
                }
            }
            break;

        case '\n':
            if (strcmp(cfg->eol, "LF") != 0)
            {
                fprintf(stderr, "LF found on line %u in file `%s`.\n", (unsigned)cnt_line, cfg->path);
                mmc_dump_hex(file->data + line_offset, line_sz + 1, 16);
                ret = EXIT_FAILURE;
                goto fin;
            }
            break;

        default:
            fprintf(stderr, "unexcept character `%c`.\n", file->data[eol_offset]);
            abort();
        }
    }

fin:
    _eolcheck_release_file(&cpy);
    return ret;
}

static int tool_eolcheck(int argc, char* argv[])
{
    int ret;

    eolcheck_cfg_t cfg;
    if ((ret = _eolcheck_get_config(&cfg, argc, argv)) != 0)
    {
        return ret;
    }

    eolcheck_file_t file = EOLCHECK_FILE_INIT;
    if ((ret = _eolcheck_read_file(&file, cfg.path)) != 0)
    {
        goto fin;
    }

    ret = _eolcheck_check_ending(&file, &cfg);

fin:
    _eolcheck_release_file(&file);
    return ret;
}

test_tool_t test_tool_eolcheck = {
    "eolcheck", tool_eolcheck,
    "Check if a file contains all line ending.\n"
    "--file=[PATH]\n"
    "    Path to check.\n"
    "--eol=CR|LF|CRLF\n"
    "    Excepet line ending. CR (Macintosh) / LF (Unix) / CRLF (Windows)"
};
