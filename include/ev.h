/**
 * @mainpage libev
 *
 * \section EV_OVERVIEW Overview
 *
 * [libev](https://github.com/qgymib/libev) is a rework of
 * [libuv](https://github.com/libuv/libuv), with following advantages:
 * 1. Strong types without static casts any more.
 * 2. Enhanced IPC features.
 * 3. Easy to use file system operations.
 *
 *
 * \section EV_DOCUMENTATION Documentation
 *
 * Located in the docs/ subdirectory. It use [Doxygen](http://www.doxygen.nl/)
 * to build documents, which means the source code is well documented and can
 * be read directly without build it.
 *
 * To build html-based documents, in the project root, run:
 * ```bash
 * doxygen docs/Doxyfile
 * ```
 *
 * Also documents can be browsed online [here](https://qgymib.github.io/libev/).
 *
 *
 * \section EV_BUILD_INSTRUCTIONS Build Instructions
 *
 * [CMake](https://cmake.org/) is currently the prefer way to build:
 *
 * ```bash
 * # Build:
 * $ mkdir -p build
 * $ (cd build && cmake .. -DBUILD_TESTING=ON) # generate project with tests
 * $ cmake --build build                       # add `-j <n>` with cmake >= 3.12
 * 
 * # Run tests:
 * $ (cd build && ctest -C Debug --output-on-failure)
 * ```
 *
 * \section EV_AMALGAMATE Amalgamate
 * 
 * [libev](https://github.com/qgymib/libev) support amalgamate build, which
 * allow to distribute libev's source code using only two files(`ev.h` and `ev.c`).
 *
 * > Note: Amalgamate requires python3.
 *
 * To use amalgamation, add `-DEV_AMALGAMATE_BUILD=on` when configurate cmake:
 *
 * ```bash
 * $ cmake -DEV_AMALGAMATE_BUILD=on /path/to/libev
 * $ cmake --build .
 * ```
 *
 * In `${CMAKE_CURRENT_BINARY_DIR}/amalgamate` directory, you will find the
 * generated files.
 *
 */
#ifndef __EV_H__
#define __EV_H__

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdarg.h>

#include "ev/expose.h"
#include "ev/version.h"
#include "ev/list.h"
#include "ev/map.h"
#include "ev/defines.h"
#include "ev/errno.h"
#include "ev/allocator.h"
#include "ev/queue.h"

#if defined(_WIN32)
#   include "ev/win.h"
#else
#   include "ev/unix.h"
#endif

#include "ev/atomic.h"
#include "ev/thread.h"
#include "ev/request.h"
#include "ev/mutex.h"
#include "ev/sem.h"
#include "ev/once.h"
#include "ev/shm.h"
#include "ev/shdlib.h"
#include "ev/time.h"
#include "ev/handle.h"
#include "ev/loop.h"
#include "ev/async.h"
#include "ev/timer.h"
#include "ev/tcp.h"
#include "ev/udp.h"
#include "ev/pipe.h"
#include "ev/fs.h"
#include "ev/process.h"
#include "ev/misc.h"

#endif
