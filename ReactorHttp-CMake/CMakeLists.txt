# cmake版本号
cmake_minimum_required(VERSION 3.10.2)
# 工程名
project(DDZ_Server)
# c++标准
set(CMAKE_CXX_STANDARD 17)

# 给目标提供头文件, ${PROJECT_SOURCE_DIR}当前目标的根目录
include_directories(
        ${PROJECT_SOURCE_DIR}/common
        ${PROJECT_SOURCE_DIR}/http
        ${PROJECT_SOURCE_DIR}/reactor
        ${PROJECT_SOURCE_DIR}/tcp
        ${PROJECT_SOURCE_DIR}/thread
)

# 添加子目录
add_subdirectory(common)
add_subdirectory(http)
add_subdirectory(reactor)
add_subdirectory(tcp)
add_subdirectory(thread)

# 链接子目录中生成的静态库文件
link_libraries(tcp
               http
               reactor
               thread
               common)

# 指定可执行文件生成到对应的目录中
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 编译出可执行文件DDZ_Server
add_executable(DDZ_Server main.cpp)
