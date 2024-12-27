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

size_t ev_os_mmap_offset_granularity(void)
{
    return ev_os_page_size();
}

void ev__backend_shutdown(void)
{
    ev__exit_process_unix();
}
