# 设置测试程序名称
set(TEST_NAME TestShareMemory)

# 添加测试程序
add_executable(${TEST_NAME} TestShareMemory.cpp)

# 添加依赖
if (BUILD_SHARED_REACTOR_LIB)
    add_dependencies(${TEST_NAME} ${REACTOR_LIB_SHARED})
else()
    add_dependencies(${TEST_NAME} ${REACTOR_LIB_STATIC})
endif()

# 链接库
target_link_directories(${TEST_NAME} PRIVATE ${REACTOR_LIBRARY_PATH})
target_link_libraries(${TEST_NAME} PRIVATE ${REACTOR_LIB_NAME})
target_include_directories(${TEST_NAME} PRIVATE ${REACTOR_INCLUDE_PATH})