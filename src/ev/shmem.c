
void* ev_shmem_addr(ev_shmem_t* shm)
{
    return shm->addr;
}

size_t ev_shmem_size(ev_shmem_t* shm)
{
    return shm->size;
}
