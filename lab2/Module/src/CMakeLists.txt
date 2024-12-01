#设置头文件包含路径：宏CMAKE_SOURCE_DIR指向顶层CMakeLists.txt位置，头文件在其子文件include下
#在顶层CMakeLists.txt已经设置过，这里可以不用
#include_directories(${CMAKE_SOURCE_DIR}/include)

#将当前目录中所有.cpp文件名放入SOURCE变量中，除去Main.cpp和main.cpp（main文件不编译为库）
FILE(GLOB_RECURSE SOURCE "*.cpp")
LIST(REMOVE_ITEM SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp)
LIST(REMOVE_ITEM SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

ADD_COMPILE_OPTIONS("-Wall")
ADD_COMPILE_OPTIONS("-Werror")

#将src中的cpp制作为lib库，默认路径为build目录
#设置静态库输出路径
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

#生成名为src的lib库 cmke会在指定目录中生成libsrc.a静态库
ADD_LIBRARY(src ${SOURCE})

