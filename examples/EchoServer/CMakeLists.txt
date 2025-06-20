# 设置示例程序名称
set(EXAMPLE_NAME EchoServer)

# 添加示例程序
add_executable(${EXAMPLE_NAME} EchoServer.cpp)

# 添加依赖
if (BUILD_SHARED_REACTOR_LIB)
    add_dependencies(${EXAMPLE_NAME} ${REACTOR_LIB_SHARED})
else()
    add_dependencies(${EXAMPLE_NAME} ${REACTOR_LIB_STATIC})
endif()

# 链接库
target_link_directories(${EXAMPLE_NAME} PRIVATE ${REACTOR_LIBRARY_PATH})
target_link_libraries(${EXAMPLE_NAME} PRIVATE ${REACTOR_LIB_NAME})
target_include_directories(${EXAMPLE_NAME} PRIVATE ${REACTOR_INCLUDE_PATH})