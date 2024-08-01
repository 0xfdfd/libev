cmake_minimum_required(VERSION 3.12)
project(ev)

# add custom cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party/cmake-modules")

# Build options
option(EV_ENABLE_COVERAGE "Enable code coverage. GCC/Clang only" OFF)
option(EV_DEV "Enable develop mode." OFF)

###############################################################################
# Support functions
###############################################################################
function(ev_setup_target_wall name)
    if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${name} PRIVATE /W4 /WX)
    else ()
        target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
    endif ()
endfunction()

###############################################################################
# Dev Mode
###############################################################################

if (EV_DEV)
    add_library(${PROJECT_NAME}_raw
        "src/ev.c"
    )
    target_include_directories(${PROJECT_NAME}_raw
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    ev_setup_target_wall(${PROJECT_NAME}_raw)

    add_subdirectory(tool/amalgamate)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ev.h
        COMMAND $<TARGET_FILE:amalgamate>
                --in=${CMAKE_CURRENT_SOURCE_DIR}/include/ev.h
                --out=${CMAKE_CURRENT_SOURCE_DIR}/ev.h
                --commit=${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
                --commit=${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
                ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
                ${PROJECT_NAME}_raw
        VERBATIM
    )
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ev.c
        COMMAND $<TARGET_FILE:amalgamate>
                --in=${CMAKE_CURRENT_SOURCE_DIR}/src/ev.c
                --out=${CMAKE_CURRENT_SOURCE_DIR}/ev.c
                --commit=${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
                ${CMAKE_CURRENT_SOURCE_DIR}/ev.h
                ${PROJECT_NAME}_raw
        VERBATIM
    )
endif()

###############################################################################
# Build libev
###############################################################################

add_library(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/ev.c)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_property(TARGET ${PROJECT_NAME}
    PROPERTY POSITION_INDEPENDENT_CODE ON
)

# add library dependencies
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Ws2_32 Mswsock)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE rt)
endif()

# add warning check
ev_setup_target_wall(${PROJECT_NAME})

if (EV_ENABLE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "(GNU|Clang)")
    include(CodeCoverage)
    target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
    set(EV_HAVE_COVERAGE true)
endif ()

###############################################################################
# Build examples
###############################################################################
add_subdirectory(example)

###############################################################################
# Build test
###############################################################################
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    include(third_party/cutest.cmake)
    include(test/CMakeLists.txt)
endif()
