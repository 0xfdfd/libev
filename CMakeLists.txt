cmake_minimum_required(VERSION 3.12)
project(ev)

# add custom cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party/cmake-modules")

# Build options
option(EV_ENABLE_COVERAGE "Enable code coverage. GCC/Clang only" OFF)
option(EV_AMALGAMATE_BUILD "Build amalgamate source file" OFF)
option(EV_HAVE_LUA_BINDING "Build with lua binding" OFF)

set(ev_header_public_list
    "include/ev.h"
    "include/ev/win.h"
    "include/ev/unix.h"
)

set(ev_src_hdr
    "src/defs.h"
    "src/allocator.h"
    "src/async.h"
    "src/handle.h"
    "src/loop.h"
    "src/fs.h"
    "src/misc.h"
    "src/pipe.h"
    "src/ringbuffer.h"
    "src/threadpool.h"
    "src/timer.h"
    "src/log.h"
    "src/udp.h"
)

set(ev_src_hdr_os_win
    "src/win/winapi.h"
    "src/win/winsock.h"
    "src/win/async_win.h"
    "src/win/fs_win.h"
    "src/win/udp_win.h"
    "src/win/loop_win.h"
    "src/win/process_win.h"
    "src/win/pipe_win.h"
    "src/win/misc_win.h"
    "src/win/thread_win.h"
    "src/win/threadpool_win.h"
    "src/win/tcp_win.h"
)

set(ev_src_hdr_os_unix
    "src/unix/async_unix.h"
    "src/unix/io_unix.h"
    "src/unix/process_unix.h"
    "src/unix/tcp_unix.h"
    "src/unix/loop_unix.h"
    "src/unix/misc_unix.h"
    "src/unix/stream_unix.h"
    "src/unix/work.h"
)

set(ev_src
    "src/allocator.c"
    "src/fs.c"
    "src/handle.c"
    "src/list.c"
    "src/log.c"
    "src/loop.c"
    "src/map.c"
    "src/misc.c"
    "src/pipe.c"
    "src/queue.c"
    "src/ringbuffer.c"
    "src/shmem.c"
    "src/threadpool.c"
    "src/timer.c"
    "src/udp.c"
    "src/version.c"
)

set(ev_src_os_win
    "src/win/async_win.c"
    "src/win/fs_win.c"
    "src/win/loop_win.c"
    "src/win/misc_win.c"
    "src/win/mutex_win.c"
    "src/win/once_win.c"
    "src/win/pipe_win.c"
    "src/win/process_win.c"
    "src/win/sem_win.c"
    "src/win/shmem_win.c"
    "src/win/tcp_win.c"
    "src/win/thread_win.c"
    "src/win/threadpool_win.c"
    "src/win/udp_win.c"
    "src/win/winapi.c"
    "src/win/winsock.c"
)

set(ev_src_os_unix
    "src/unix/async_unix.c"
    "src/unix/fs_unix.c"
    "src/unix/io_unix.c"
    "src/unix/loop_unix.c"
    "src/unix/misc_unix.c"
    "src/unix/mutex_unix.c"
    "src/unix/once_unix.c"
    "src/unix/pipe_unix.c"
    "src/unix/process_unix.c"
    "src/unix/sem_unix.c"
    "src/unix/shmem_unix.c"
    "src/unix/stream_unix.c"
    "src/unix/tcp_unix.c"
    "src/unix/thread_unix.c"
    "src/unix/threadpool_unix.c"
    "src/unix/udp_unix.c"
)

function(build_with_native name)
    add_library(${name} ${ev_src})
    if(WIN32)
        target_sources(${PROJECT_NAME} PRIVATE ${ev_src_os_win})
    else()
        target_sources(${PROJECT_NAME} PRIVATE ${ev_src_os_unix})
    endif()
    target_include_directories(${name}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src)
endfunction()

function(build_with_amalgamate name)
    find_package (Python3 REQUIRED COMPONENTS Interpreter)

    string(REPLACE ";" "," escaped_ev_header_public_list      "${ev_header_public_list}")
    string(REPLACE ";" "," escaped_ev_src_hdr         "${ev_src_hdr}")
    string(REPLACE ";" "," escaped_ev_src_hdr_os_win  "${ev_src_hdr_os_win}")
    string(REPLACE ";" "," escaped_ev_src_hdr_os_unix "${ev_src_hdr_os_unix}")
    string(REPLACE ";" "," escaped_ev_src             "${ev_src}")
    string(REPLACE ";" "," escaped_ev_src_os_win      "${ev_src_os_win}")
    string(REPLACE ";" "," escaped_ev_src_os_unix     "${ev_src_os_unix}")

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/amalgamate/ev.h ${CMAKE_CURRENT_BINARY_DIR}/amalgamate/ev.c
        COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.py
                --out ${CMAKE_CURRENT_BINARY_DIR}/amalgamate
                --public_hdr        ${escaped_ev_header_public_list}
                --src_hdr           ${escaped_ev_src_hdr}
                --src_hdr_os_win    ${escaped_ev_src_hdr_os_win}
                --src_hdr_os_unix   ${escaped_ev_src_hdr_os_unix}
                --src               ${escaped_ev_src}
                --src_os_win        ${escaped_ev_src_os_win}
                --src_os_unix       ${escaped_ev_src_os_unix}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${ev_header_public_list}
                ${ev_src_hdr}
                ${ev_src_hdr_os_win}
                ${ev_src_hdr_os_unix}
                ${ev_src}
                ${ev_src_os_win}
                ${ev_src_os_unix})
    add_library(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/amalgamate/ev.c)
    target_include_directories(${name}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/amalgamate>)
endfunction()

# add source files
if (EV_AMALGAMATE_BUILD)
    build_with_amalgamate(${PROJECT_NAME})
else ()
    build_with_native(${PROJECT_NAME})
endif ()

# add library dependencies
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Ws2_32 Mswsock)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE rt)
endif()

function(ev_setup_target_wall name)
    if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${name} PRIVATE /W4 /WX)
    else ()
        target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
    endif ()
endfunction()

# add warning check
ev_setup_target_wall(${PROJECT_NAME})

if (EV_ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "(GNU|Clang)")
    include(CodeCoverage)
    target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
    set(EV_HAVE_COVERAGE true)
endif ()

# build examples
add_subdirectory(example)

# build test
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(third_party/cutest)
    include(test/CMakeLists.txt)
endif()

if (EV_HAVE_LUA_BINDING)
    include(lua/CMakeLists.txt)
endif ()
