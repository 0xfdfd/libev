cmake_minimum_required(VERSION 3.2)
project(ev)

option(EV_PGO_EXPORT "Export Profile Guided Optimization" OFF)
option(EV_PGO_IMPORT "Import Profile Guided Optimization" OFF)

# add source files
add_library(${PROJECT_NAME}
    "src/ev-common.c"
    "src/list.c"
    "src/log.c"
    "src/map.c"
    "src/map_low.c"
    "src/timer.c")
if(WIN32)
    target_sources(${PROJECT_NAME}
        PRIVATE
            "src/win/async.c"
            "src/win/loop.c"
            "src/win/once.c"
            "src/win/pipe.c"
            "src/win/tcp.c"
            "src/win/winapi.c")
else()
    target_sources(${PROJECT_NAME}
        PRIVATE
            "src/unix/async.c"
            "src/unix/loop.c"
            "src/unix/once.c"
            "src/unix/pipe.c"
            "src/unix/tcp.c")
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

# build test
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(third_party/cutest)
    add_subdirectory(test)
endif()
