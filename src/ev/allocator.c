#include <stdlib.h>
#include <string.h>

static void *_ev_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

static ev_realloc_fn s_allocator = _ev_realloc;

int ev_replace_allocator(ev_realloc_fn  new_allocator,
                         ev_realloc_fn *old_allocator)
{
    if (new_allocator == NULL)
    {
        return EV_EINVAL;
    }

    if (old_allocator != NULL)
    {
        *old_allocator = s_allocator;
    }
    s_allocator = new_allocator;

    return 0;
}

void *ev_calloc(size_t nmemb, size_t size)
{
    const size_t alloc_size = size * nmemb;
    void        *ptr = s_allocator(NULL, size * nmemb);
    if (ptr != NULL)
    {
        memset(ptr, 0, alloc_size);
    }
    return ptr;
}

void *ev_malloc(size_t size)
{
    return s_allocator(NULL, size);
}

void *ev_realloc(void *ptr, size_t size)
{
    return s_allocator(ptr, size);
}

void ev_free(void *ptr)
{
    s_allocator(ptr, 0);
}

char *ev__strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char  *m = ev_malloc(len);
    if (m == NULL)
    {
        return NULL;
    }
    return memcpy(m, s, len);
}
