cmake_minimum_required(VERSION 3.2)
project(ev)

# add source files
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_C)
if(WIN32)
	aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/win SRC_C)
else()
	aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/unix SRC_C)
endif()
add_library(${PROJECT_NAME} ${SRC_C})

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
