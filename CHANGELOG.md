# Changelog

## v0.0.8

### BREAKING CHANGES

### Features
1. add detail information for `ev_fs_readfile()`

### Bug Fixes


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
