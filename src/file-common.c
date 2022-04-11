#include "ev-common.h"

ev_file_stat_t* ev_fs_get_statbuf(ev_fs_req_t* req)
{
    return &req->rsp.as_fstat.fileinfo;
}
