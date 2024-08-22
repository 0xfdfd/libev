#include <errno.h>
#include <unistd.h>

EV_LOCAL int ev__translate_sys_error(int syserr)
{
    return ev__translate_posix_sys_error(syserr);
}

size_t ev_os_page_size(void)
{
    return sysconf(_SC_PAGE_SIZE);
}
