cmake_minimum_required(VERSION 3.2)
project(ev)

# add custom cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party/cmake-modules")

option(EV_PGO_EXPORT "Export Profile Guided Optimization" OFF)
option(EV_PGO_IMPORT "Import Profile Guided Optimization" OFF)
option(EV_ENABLE_COVERAGE "Enable code coverage. GCC/Clang only" OFF)

# add source files
add_library(${PROJECT_NAME}
    "src/allocator.c"
    "src/fs.c"
    "src/handle.c"
    "src/list.c"
    "src/log.c"
    "src/loop.c"
    "src/map.c"
    "src/map_low.c"
    "src/misc.c"
    "src/pipe.c"
    "src/queue.c"
    "src/ringbuffer.c"
    "src/shmem.c"
    "src/threadpool.c"
    "src/timer.c"
    "src/todo.c"
    "src/udp.c"
    "src/version.c")
if(WIN32)
    target_sources(${PROJECT_NAME}
        PRIVATE
            "src/win/async_win.c"
            "src/win/fs_win.c"
            "src/win/loop_win.c"
            "src/win/mutex_win.c"
            "src/win/once_win.c"
            "src/win/pipe_win.c"
            "src/win/process_win.c"
            "src/win/sem_win.c"
            "src/win/shmem_win.c"
            "src/win/tcp_win.c"
            "src/win/udp_win.c"
            "src/win/thread_win.c"
            "src/win/winapi.c"
            "src/win/work_win.c")
else()
    target_sources(${PROJECT_NAME}
        PRIVATE
            "src/unix/async_unix.c"
            "src/unix/fs_unix.c"
            "src/unix/io_unix.c"
            "src/unix/loop_unix.c"
            "src/unix/mutex_unix.c"
            "src/unix/once_unix.c"
            "src/unix/pipe_unix.c"
            "src/unix/process_unix.c"
            "src/unix/sem_unix.c"
            "src/unix/shmem_unix.c"
            "src/unix/stream_unix.c"
            "src/unix/tcp_unix.c"
            "src/unix/thread_unix.c"
            "src/unix/udp_unix.c"
            "src/unix/work_unix.c")
endif()

# add library dependencies
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads)
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Ws2_32
            Mswsock)
else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            rt)
endif()

# define target include search paths
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

# add warning check
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif ()

# PGO
if (EV_PGO_IMPORT)
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME}
            PRIVATE
                -fprofile-use=$<TARGET_FILE_DIR:${PROJECT_NAME}>)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    endif ()
elseif (EV_PGO_EXPORT)
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${PROJECT_NAME}
            PRIVATE
                -fprofile-generate=$<TARGET_FILE_DIR:${PROJECT_NAME}>)
        target_link_libraries(${PROJECT_NAME}
            PRIVATE
                gcov)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    endif ()
endif ()

if (EV_ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "(GNU|Clang)")
    include(CodeCoverage)
    target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
    set(EV_HAVE_COVERAGE true)
endif ()

# build test
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(third_party/cutest)
    add_subdirectory(test)
endif()
