# Changelog

## v0.0.9

### BREAKING CHANGES
1. `ev_map_insert()` now return conflict node address
2. lua: loop:fs_file now open with RDONLY if no options specificed

### Features
1. add lua bindings

### Bug Fixes
1. fix: loop may wait infinite with EV_LOOP_MODE_ONCE if there are only endgame events
2. fix: memory leak in create process
3. fix: thread leak when process exit
4. fix: wrong format charster in test


## v0.0.8 (2023/04/07)

### BREAKING CHANGES
1. remove `ev_todo_token_t`
2. `ev_process_t` must be exited manually
3. rename position read and write to `pread` and `pwrite`
4. embed default thread pool
5. change thread pool functions
6. expose peeraddr in udp recv callback
7. reduce parameter in udp send callback

### Features
1. add detail information for `ev_fs_readfile()`
2. use `ev_library_shutdown()` to cleanup global resources
3. use `ev_loop_walk()` to walk through all handles
4. process: support change cwd
5. add amalgamate support
6. submit threadpool task by loop api
7. update test framework to v3.0.0
8. use `ev_hrtime()` to get high-resolution time
9. support normal file read / write / seek operations
10. support remove file and directory
11. expose memory allocate functions
12. redone error number
13. smart convert addr and name

### Bug Fixes
1. fix: use uninitialised value in `ev_exepath()` on Unix
2. fix: thread pool not unlink when exit loop
3. fix: `ev_process_spawn()` coredump on windows when redirect file


## v0.0.7 (2022/06/07)

### Features
1. Support `ev_getcwd()`
2. Support `ev_exepath()`

### Bug Fixes
1. fix: use of enum without previous declaration
2. fix: `FindFirstFile()` handle leak
3. fix: `ev_fs_readdir()` not working on windows


## v0.0.6 (2022/05/23)

### BREAKING CHANGES
1. change version code rule.
2. remove return value for `ev_mutex_init()`.
3. remove return value for `ev_sem_init()`.

### Features
1. Support unlink threadpool
2. publish `ev_todo_submit()`

### Bug Fixes
1. fix: crash when child process exit


## v0.0.5 (2022/05/13)

### BREAKING CHANGES
1. rename `EV_THREAD_WAIT_INFINITE` to `EV_INFINITE_TIMEOUT`.
2. `ev_pipe_make()` now have flags.

### Features
1. Support mkdir
2. Support process
3. file: synchronous operations


## v0.0.4 (2022/04/12)

### Bug Fixes
1. build error with glibc version lower than `2.28`


## v0.0.3 (2022/04/12)

### Features
1. Add version code
2. add file support


## v0.0.2 (2022/03/10)

### Features
1. ThreadPool: is able to link to event loop
2. List: support migrate

### Bug Fixes
1. test failure due to incorrect test sequence


## v0.0.1 (2022/02/22)

Initial release
