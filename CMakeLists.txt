cmake_minimum_required(VERSION 3.12)
project(ev)

# add custom cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party/cmake-modules")

# Build options
option(EV_ENABLE_COVERAGE "Enable code coverage. GCC/Clang only" OFF)
option(EV_HAVE_LUA_BINDING "Build with lua binding" OFF)

set(ev_src
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
    "src/unix/async_unix.h"
    "src/unix/io_unix.h"
    "src/unix/process_unix.h"
    "src/unix/tcp_unix.h"
    "src/unix/loop_unix.h"
    "src/unix/misc_unix.h"
    "src/unix/stream_unix.h"
    "src/unix/work.h"
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

set(ev_src_lua
    "lua/src/ev.lua.internal.h"
    "lua/src/channel.lua.h"
    "lua/src/fs.lua.h"
    "lua/src/misc.lua.h"
    "lua/src/pipe.lua.h"
    "lua/src/process.lua.h"
    "lua/src/promise.lua.h"
    "lua/src/tcp.lua.h"
    "lua/src/timer.lua.h"
    "lua/src/udp.lua.h"
    "lua/src/ev.lua.c"
    "lua/src/channel.lua.c"
    "lua/src/fs.lua.c"
    "lua/src/misc.lua.c"
    "lua/src/pipe.lua.c"
    "lua/src/process.lua.c"
    "lua/src/promise.lua.c"
    "lua/src/tcp.lua.c"
    "lua/src/timer.lua.c"
    "lua/src/udp.lua.c"
)

include(third_party/lua54.cmake)

string(REPLACE ";" "," escaped_ev_src                   "${ev_src}")
string(REPLACE ";" "," escaped_ev_src_os_win            "${ev_src_os_win}")
string(REPLACE ";" "," escaped_ev_src_os_unix           "${ev_src_os_unix}")
string(REPLACE ";" "," escaped_ev_src_lua               "${ev_src_lua}")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ev.h
    COMMAND $<TARGET_FILE:lua54> ${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.lua
            --no_line
            --out       ${CMAKE_CURRENT_SOURCE_DIR}/ev.h
            --commit    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
            --commit    ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
            --source    "include/ev/expose.h"
            --source    "include/ev/version.h"
            --source    "include/ev/list.h"
            --source    "include/ev/map.h"
            --string    "#if defined(_WIN32) /* AMALGAMATE: ev.h (1/3) */"
            --source    "include/ev/win.h"
            --string    "#else               /* AMALGAMATE: ev.h (2/3) */"
            --source    "include/ev/unix.h"
            --string    "#endif              /* AMALGAMATE: ev.h (3/3) */"
            --source    "include/ev.h"
            --source    "lua/include/ev.lua.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.lua
            ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
            ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
            ${CMAKE_CURRENT_SOURCE_DIR}/include/ev/expose.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/ev/version.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/ev/list.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/ev/map.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/ev.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/ev/win.h
            ${CMAKE_CURRENT_SOURCE_DIR}/include/ev/unix.h
            ${CMAKE_CURRENT_SOURCE_DIR}/lua/include/ev.lua.h
    VERBATIM
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ev.c
    COMMAND $<TARGET_FILE:lua54> ${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.lua
            --out               ${CMAKE_CURRENT_SOURCE_DIR}/ev.c
            --commit            ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
            --string            "#define EV_AMALGAMATE_BUILD"
            --string            "#define EV_EXPOSE_SYMBOLS"
            --string            "#define _GNU_SOURCE"
            --string            "#include \"ev.h\""
            --source            ${escaped_ev_src}
            --string            "#if defined(_WIN32) /* AMALGAMATE: ev.c (1/3) */"
            --source            ${escaped_ev_src_os_win}
            --string            "#else               /* AMALGAMATE: ev.c (2/3) */"
            --source            ${escaped_ev_src_os_unix}
            --string            "#endif              /* AMALGAMATE: ev.c (3/3) */"
            --string            "#if defined(EV_HAVE_LUA_BINDING) /* FEATURE: EV_HAVE_LUA_BINDING (1/2) */"
            --string            "#include \"lua.h\""
            --string            "#include \"lauxlib.h\""
            --source            ${escaped_ev_src_lua}
            --string            "#endif                           /* FEATURE: EV_HAVE_LUA_BINDING (2/2) */"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.lua
            ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
            ${CMAKE_CURRENT_SOURCE_DIR}/ev.h
            ${ev_src}
            ${ev_src_os_win}
            ${ev_src_os_unix}
            ${ev_src_lua}
    VERBATIM
)

add_library(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/ev.c)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

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
