project(tdcf_test)

FILE(GLOB_RECURSE SRC_LIST "*.cpp")

FILE(GLOB_RECURSE HEAD_LIST "*.hpp")

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEAD_LIST})

target_link_libraries(${PROJECT_NAME} tdcf)
