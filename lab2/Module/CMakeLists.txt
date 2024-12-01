#需求的最低cmake程序版本
cmake_minimum_required(VERSION 3.12)

#工程名
project(test)

#本工程支持的C++版本
set(CMAKE_CXX_STANDARD 17)

#将子目录src加入编译列表
#ADD_SUBDIRECTORY(src)

#声明头文件位置.
include_directories(${CMAKE_SOURCE_DIR}/include)

#将路径中全部cpp文件添加到SRCS变量中，这里通常是main.cpp
aux_source_directory(${CMAKE_SOURCE_DIR} SRCS)

#将dll文件拷贝到exe文件目录中
file(GLOB DLLSUORCE ${CMAKE_SOURCE_DIR}/bin/freeglut.dll)
file(COPY ${DLLSUORCE} DESTINATION ${CMAKE_BINARY_DIR}/Release)

#指出lib文件位置，有几个lib文件就有几个位置，系统lib可以不提供位置 不同lib文件位置用空格隔开
link_directories(${CMAKE_SOURCE_DIR}/lib)

#生成名为test的可执行文件test.exe
add_executable(test ${SRCS})

#将静态链接库链接到可执行文件,src是子目录src生成的链接库opengl32 glu32是MinGW环境下编译OpenGL必要的系统库
target_link_libraries(test freeglut opengl32 glu32)