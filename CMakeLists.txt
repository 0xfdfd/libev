cmake_minimum_required(VERSION 3.2)
project(ev)

# add source files
add_library(${PROJECT_NAME}
    "src/ev-common.c"
    "src/list.c"
    "src/map.c"
    "src/map_low.c"
    "src/timer.c")
if(WIN32)
    target_sources(${PROJECT_NAME}
        PRIVATE
            "src/win/async.c"
            "src/win/loop.c"
            "src/win/once.c"
            "src/win/tcp.c")
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
        Threads::Threads
)
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Ws2_32
            Mswsock
    )
else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            rt
    )
endif()

# define target include search paths
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# add warning check
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif ()

# build test
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(third_party/cutest)
    add_subdirectory(test)
endif()
