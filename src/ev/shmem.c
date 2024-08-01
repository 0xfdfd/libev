
void* ev_shm_addr(ev_shm_t* shm)
{
    return shm->addr;
}

size_t ev_shm_size(ev_shm_t* shm)
{
    return shm->size;
}
