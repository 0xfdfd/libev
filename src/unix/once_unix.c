#include "ev.h"
#include <stdlib.h>

void ev_once_execute(ev_once_t* guard, ev_once_cb cb)
{
    if (pthread_once(&guard->guard, cb) != 0)
    {
        abort();
    }
}
