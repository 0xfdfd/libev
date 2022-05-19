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
    "src/async.c"
    "src/ev-common.c"
    "src/fs-common.c"
    "src/handle.c"
    "src/list.c"
    "src/log.c"
    "src/map.c"
    "src/map_low.c"
    "src/pipe-common.c"
    "src/queue.c"
    "src/ringbuffer.c"
    "src/shmem-common.c"
    "src/threadpool.c"
    "src/timer.c"
    "src/todo.c"
    "src/udp-common.c"
    "src/utils.c"
    "src/version.c")
if(WIN32)
    target_sources(${PROJECT_NAME}
        PRIVATE
            "src/win/fs.c"
            "src/win/loop.c"
            "src/win/mutex.c"
            "src/win/once.c"
            "src/win/pipe.c"
            "src/win/process.c"
            "src/win/sem.c"
            "src/win/shmem.c"
            "src/win/tcp.c"
            "src/win/udp.c"
            "src/win/thread.c"
            "src/win/winapi.c")
else()
    target_sources(${PROJECT_NAME}
        PRIVATE
            "src/unix/fs.c"
            "src/unix/io.c"
            "src/unix/loop.c"
            "src/unix/mutex.c"
            "src/unix/once.c"
            "src/unix/pipe.c"
            "src/unix/process.c"
            "src/unix/sem.c"
            "src/unix/shmem.c"
            "src/unix/stream.c"
            "src/unix/tcp.c"
            "src/unix/thread.c"
            "src/unix/udp.c")
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
