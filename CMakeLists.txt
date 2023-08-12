CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
set(CMAKE_INSTALL_PREFIX /home/wjxh)

# -g 不加没有行号
# -rdynamic 不加没有函数名
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -rdynamic -Wl,-rpath=.")
project(app)

add_subdirectory(backtrace)
