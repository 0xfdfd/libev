#include <stdlib.h>
#include "str.h"

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

char* _eolcheck_strtok(char* str, const char* delim, char** saveptr)
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
