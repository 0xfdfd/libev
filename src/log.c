#include "log.h"
#include <stdarg.h>
#include <stdio.h>

static const char* _ev_filename(const char* file)
{
    const char* pos = file;

    for (; *file; ++file)
    {
        if (*file == '\\' || *file == '/')
        {
            pos = file + 1;
        }
    }
    return pos;
}

static char _ev_ascii_to_char(unsigned char c)
{
    if (c >= 32 && c <= 126)
    {
        return c;
    }
    return '.';
}

void ev__log(ev_log_level_t level, const char* file, const char* func,
    int line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    const char* prefix;
    switch (level)
    {
    case EV_LOG_INFO:
        prefix = "I";
        break;
    case EV_LOG_ERROR:
        prefix = "E";
        break;
    case EV_LOG_FATAL:
        prefix = "F";
        break;
    default:
        prefix = "T";
        break;
    }

    printf("[%s %s:%d %s] ", prefix, _ev_filename(file), line, func);
    vprintf(fmt, ap);
    printf("\n");

    va_end(ap);
}

void ev__dump_hex(const void* data, size_t size, size_t width)
{
    const unsigned char* pdat = (unsigned char*)data;

    size_t idx_line;
    for (idx_line = 0; idx_line < size; idx_line += width)
    {
        size_t idx_colume;
        /* printf hex */
        for (idx_colume = 0; idx_colume < width; idx_colume++)
        {
            const char* postfix = (idx_colume < width - 1) ? "" : "|";

            if (idx_colume + idx_line < size)
            {
                fprintf(stdout, "%02x %s", pdat[idx_colume + idx_line], postfix);
            }
            else
            {
                fprintf(stdout, "   %s", postfix);
            }
        }
        fprintf(stdout, " ");
        /* printf char */
        for (idx_colume = 0; (idx_colume < width) && (idx_colume + idx_line < size); idx_colume++)
        {
            fprintf(stdout, "%c", _ev_ascii_to_char(pdat[idx_colume + idx_line]));
        }
        fprintf(stdout, "\n");
    }

}
