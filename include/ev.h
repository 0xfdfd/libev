/**
 * @mainpage libev
 *
 * \section Overview
 *
 * libev is a rework of [libuv](https://github.com/libuv/libuv), with following
 * advantages:
 * 1. Strong types without static casts any more.
 * 2. Enhanced IPC features.
 * 3. Easy to use file system operations.
 *
 *
 * \section Documentation
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
 * \section Build Instructions
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
 */
#ifndef __EV_H__
#define __EV_H__

#include "ev/allocator.h"
#include "ev/async.h"
#include "ev/buf.h"
#include "ev/errno.h"
#include "ev/fs.h"
#include "ev/handle.h"
#include "ev/list.h"
#include "ev/loop.h"
#include "ev/map.h"
#include "ev/misc.h"
#include "ev/mutex.h"
#include "ev/once.h"
#include "ev/pipe.h"
#include "ev/process.h"
#include "ev/queue.h"
#include "ev/sem.h"
#include "ev/shmem.h"
#include "ev/tcp.h"
#include "ev/thread.h"
#include "ev/threadpool.h"
#include "ev/time.h"
#include "ev/timer.h"
#include "ev/udp.h"
#include "ev/version.h"

#endif
