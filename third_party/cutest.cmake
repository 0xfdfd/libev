set(CUTEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cutest)

add_library(cutest
    "${CUTEST_ROOT}/cutest.c"
)
target_include_directories(cutest
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CUTEST_ROOT}>
    PRIVATE
        ${CUTEST_ROOT}
)
set_property(TARGET cutest
    PROPERTY POSITION_INDEPENDENT_CODE ON
)
target_compile_options(cutest
    PUBLIC
        -DCUTEST_USE_DLL
)
