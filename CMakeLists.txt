# cmake版本要求
cmake_minimum_required(VERSION 3.22)

# 工程名称
project(Reactor)

# 设置编译器
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置为debug可调试
set(CMAKE_BUILD_TYPE Debug)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)

# 设置静态库/动态库编译开启选项
option(BUILD_SHARED_REACTOR_LIB "Build Shared Libraries" OFF)
option(BUILD_STATIC_REACTOR_LIB "Build Static Libraries" ON)

# 设置静态库/动态库名称
set(REACTOR_LIB_NAME ${PROJECT_NAME})
set(REACTOR_LIB_STATIC "${PROJECT_NAME}Static")
set(REACTOR_LIB_SHARED "${PROJECT_NAME}Shared")

# 设置库路径和头文件路径
set(REACTOR_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/lib)
set(REACTOR_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include)

# 库输出路径
set(LIBRARY_OUTPUT_PATH ${REACTOR_LIBRARY_PATH})

# 参与编译的文件
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_FILES)

# 生成动态库/静态库
if (BUILD_SHARED_REACTOR_LIB AND BUILD_STATIC_REACTOR_LIB)
    message("Build Shared Library And Static Library Enabled")

    add_library(${REACTOR_LIB_SHARED} SHARED ${SRC_FILES})
    set_target_properties(${REACTOR_LIB_SHARED} PROPERTIES OUTPUT_NAME ${REACTOR_LIB_NAME})
    target_include_directories(${REACTOR_LIB_SHARED} PUBLIC ${REACTOR_INCLUDE_PATH})
    
    add_library(${REACTOR_LIB_STATIC} STATIC ${SRC_FILES})
    set_target_properties(${REACTOR_LIB_STATIC} PROPERTIES OUTPUT_NAME ${REACTOR_LIB_NAME})
    target_include_directories(${REACTOR_LIB_STATIC} PUBLIC ${REACTOR_INCLUDE_PATH})

elseif (BUILD_SHARED_REACTOR_LIB)
    message("Only Build Shared Library Enabled")
    
    add_library(${REACTOR_LIB_SHARED} SHARED ${SRC_FILES})
    set_target_properties(${REACTOR_LIB_SHARED} PROPERTIES OUTPUT_NAME ${REACTOR_LIB_NAME})
    target_include_directories(${REACTOR_LIB_SHARED} PUBLIC ${REACTOR_INCLUDE_PATH})

else()
    message("Only Build Static Library Enabled")

    add_library(${REACTOR_LIB_STATIC} STATIC ${SRC_FILES})
    set_target_properties(${REACTOR_LIB_STATIC} PROPERTIES OUTPUT_NAME ${REACTOR_LIB_NAME})
    target_include_directories(${REACTOR_LIB_STATIC} PUBLIC ${REACTOR_INCLUDE_PATH})

endif ()

# 测试程序
option(BUILD_TESTS "Build Tests" ON)

if (BUILD_TESTS)
    message("Build Tests Enabled")
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
else()
    message("Build Tests Disabled")
endif (BUILD_TESTS)

