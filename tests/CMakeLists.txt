# 设置可执行程序路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/tests)

# 添加测试程序目录
add_subdirectory(TestLog)
add_subdirectory(TestNetAddr)
add_subdirectory(TestBuffer)