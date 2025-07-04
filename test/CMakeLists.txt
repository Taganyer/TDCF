cmake_minimum_required(VERSION 3.31)

project(tdcf_test)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." CACHE PATH "Project root directory" FORCE)

FILE(GLOB_RECURSE SRC_LIST "*.cpp" ${CMAKE_CURRENT_SOURCE_DIR})

FILE(GLOB_RECURSE HEADER_LIST "*.hpp" ${CMAKE_CURRENT_SOURCE_DIR})

set(EXCLUDE_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/cmake-build*")

list(FILTER SRC_LIST EXCLUDE REGEX "^${EXCLUDE_PREFIX}")

list(FILTER HEADER_LIST EXCLUDE REGEX "^${EXCLUDE_PREFIX}")

message(STATUS "Header files: ${HEADER_LIST}")

message(STATUS "Source files: ${SRC_LIST}")


set(TDCF_PATH "${ROOT_DIR}/tdcf")

file(GLOB_RECURSE TDCF_HEADER_LIST "${TDCF_PATH}/*.hpp")

file(GLOB_RECURSE TDCF_SRC_LIST "${TDCF_PATH}/*.cpp")

message(STATUS "tdcf Header files: ${TDCF_HEADER_LIST}")

message(STATUS "tdcf Source files: ${TDCF_SRC_LIST}")


set(SIMULATION_PATH "${ROOT_DIR}/simulation")

file(GLOB_RECURSE SIMULATION_HEADER_LIST "${SIMULATION_PATH}/*.hpp")

file(GLOB_RECURSE SIMULATION_SRC_LIST "${SIMULATION_PATH}/*.cpp")

message(STATUS "simulation Header files: ${SIMULATION_HEADER_LIST}")

message(STATUS "simulation Source files: ${SIMULATION_SRC_LIST}")

find_package(tinyBackend REQUIRED)

add_executable(${PROJECT_NAME} ${HEADER_LIST} ${SRC_LIST}
        ${TDCF_HEADER_LIST} ${TDCF_SRC_LIST}
        ${SIMULATION_HEADER_LIST} ${SIMULATION_SRC_LIST})

target_include_directories(${PROJECT_NAME}
        PRIVATE ${ROOT_DIR}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE tinyBackend::Base)
