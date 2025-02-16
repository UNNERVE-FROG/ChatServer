

#配置头文件的搜索路径
#include_directories()

#配置库文件的搜索路径
#link_directories()

#要编译的源文件列表
set(SRC_LIST muduoserver.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#把当前目录的所有源文件添加到SRC_LIST
#aux_source_directory(. SRC_LIST)

add_executable(server ${SRC_LIST})
#需要链接的库文件
target_link_libraries(server muduo_net muduo_base pthread)
