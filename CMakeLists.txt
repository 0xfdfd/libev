cmake_minimum_required(VERSION 3.2)
project(ev)

# add source files
set(ev_sources
	src/ev-common.c
	src/list.c
	src/map.c
	src/map_low.c
	src/timer.c)
if(WIN32)
	list(APPEND ev_sources
		src/win/async.c
		src/win/loop.c
		src/win/once.c)
else()
	list(APPEND ev_sources
		src/unix/async.c
		src/unix/loop.c
		src/unix/once.c)
endif()
add_library(${PROJECT_NAME} ${ev_sources})

# add library dependencies
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Threads::Threads
)

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
	add_subdirectory(test)
endif()
