cmake_minimum_required(VERSION 3.10)
project(chat)

# 配置编译选项 compile options
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)


# 配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/)

# 链接库搜索路径
link_directories(AFTER /usr/lib/)



# 加载子目录
add_subdirectory(src)

