cmake_minimum_required(VERSION 3.31)

project(TDCF CXX)

set(CMAKE_CXX_STANDARD 17)

# 设置编译器优化级别
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g")

set(PROJECT_ROOT_NAME ${PROJECT_NAME} CACHE STRING "Project root name" FORCE)

set(PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR} CACHE PATH "Project root directory" FORCE)

set(CONFIG_PREFIX "${PROJECT_ROOT_DIR}/cmake_config" CACHE PATH "config prefix" FORCE)

set(CMAKE_BUILD_PREFIX "${PROJECT_ROOT_DIR}/build" CACHE PATH "build prefix" FORCE)

set(CMAKE_INSTALL_PREFIX "${PROJECT_ROOT_DIR}/install" CACHE PATH "Install prefix" FORCE)

#configure_file("${CONFIG_PREFIX}/CMake_config.h.in" "${PROJECT_ROOT_DIR}/CMake_config.h")

# 设置安装路径
set(INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include/ CACHE PATH "Project includes directory" FORCE)
set(INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib/ CACHE PATH "Project library directory" FORCE)
set(INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin/ CACHE PATH "Project run time directory" FORCE)
set(INSTALL_CONFIG_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/ CACHE PATH "Project cmake config directory" FORCE)

set(ADD_Testing ON CACHE BOOL "Enable testing" FORCE)
cmake_parse_arguments(
        ARG
        ""
        ""
        "ADD_Testing"
        ${CMAKE_ARGV}
)

if (ARG_ADD_Testing)
    set(ADD_Testing ${ARG_ADD_LOGSYSTEM} FORCE)
endif ()

set(tdcf_VERSION 1.0 CACHE STRING "Base Version")
add_subdirectory(tdcf)

# uninstall target
if (NOT TARGET uninstall)
    configure_file(
            "${CONFIG_PREFIX}/cmake_uninstall.cmake.in"
            "${CMAKE_BUILD_PREFIX}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)

    message(TRACE "${CMAKE_BINARY_DIR}")

    add_custom_target(uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif ()
