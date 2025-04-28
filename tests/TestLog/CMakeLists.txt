# 设置测试程序名称
set(TEST_NAME TestLog)

# 添加测试程序
add_executable(${TEST_NAME} TestLog.cpp)

# 添加依赖
add_dependencies(${TEST_NAME} ${REACTOR_LIB_STATIC})

# 链接库
target_link_directories(${TEST_NAME} PRIVATE ${REACTOR_LIBRARY_PATH})
target_link_libraries(${TEST_NAME} PRIVATE ${REACTOR_LIB_NAME})
target_include_directories(${TEST_NAME} PRIVATE ${REACTOR_INCLUDE_PATH})