# 官方CMake文件解析
**关于CMake的入门可以访问网址：https://subingwen.cn/cmake/CMake-primer/#1-CMake%E6%A6%82%E8%BF%B0**
## cmake_minimum_required(VERSION 3.5)
这行的用法是声明要求的CMake最低版本，因为旧版本的CMake对一些语法不兼容，因此需要规定要求的最低版本。
## project(ethercat_erob)
声明工程的名字
## set(CMAKE_BUILD_TYPE Debug)
设置CMake构建的模式。这里设置为Debug模式。
## set(ETHERLAB_DIR /usr/local/etherlab)
set是CMake中的一种用法，这里将/usr/local/etherlab这个路径存在ETHERLAB_DIR这个变量里。
## set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
告诉程序运行时去哪里找动态库。
## find_library(ETHERCAT_LIB ethercat HINTS ${ETHERLAB_DIR}/lib)
寻找libethercat.so或libethercat.a
## include_directories(${ETHERLAB_DIR}/include)
告诉编译器头文件去哪里找。也就是指定头文件的路径。
## add_executable(igh_driver src/igh_driver.cpp)
用于生成可执行程序，可执行程序的名称是igh_driver
```
target_include_directories(
igh_driver
  PRIVATE
  ${ETHERLAB_DIR}/include
)
```
只给`igh_driver`这个目标添加 include 路径。其实这里和前面的include_directories()重复了，只保留一个即可。这个target_include_directories是更现代的写法。
## target_link_libraries(igh_driver ${ETHERCAT_LIB})
链接`libethercat.so`