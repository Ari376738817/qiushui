cmake_minimum_required(VERSION 3.16)
project(APIManager)

# 设置C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找SQLite3
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SQLITE3 QUIET sqlite3)
endif()

if(SQLITE3_FOUND)
    include_directories(${SQLITE3_INCLUDE_DIRS})
    link_directories(${SQLITE3_LIBRARY_DIRS})
    set(SQLITE3_LIBRARIES ${SQLITE3_LIBRARIES})
else()
    # 尝试查找系统SQLite3
    find_library(SQLITE3_LIBRARIES sqlite3)
    if(NOT SQLITE3_LIBRARIES)
        message(FATAL_ERROR "SQLite3 not found. Please install SQLite3 development libraries.")
    endif()
endif()

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/main.cpp
    src/server.cpp
    src/router.cpp
    src/database.cpp
    src/utils.cpp
)

# 创建可执行文件
add_executable(api_manager ${SOURCES})

# 链接SQLite3
target_link_libraries(api_manager ${SQLITE3_LIBRARIES})

# 编译选项
if(MSVC)
    target_compile_options(api_manager PRIVATE /W4)
else()
    target_compile_options(api_manager PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 安装目标
install(TARGETS api_manager DESTINATION bin)
