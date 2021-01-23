cmake_minimum_required(VERSION 3.5)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#if (CMAKE_BUILD_TYPE STREQUAL Debug)
#set(${OUT} )
if (NOT DEFINED MS)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib/${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/bin/${CMAKE_BUILD_TYPE})
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/bin)
endif()

project(REA)

#message(":${CMAKE_BUILD_TYPE}")

add_definitions(-DEXPORTDLL)

find_package(Qt5 COMPONENTS Widgets Core Quick SerialPort SerialBus REQUIRED)

# 根据库文件代码生成动态库
add_library(REA SHARED
    "qml.qrc"
    "include/ssdp.h"
    "include/util.h"
    "include/reaC++.h"
    "include/reaQML.h"
    "include/tess.h"
    "include/storage0.h"
    "include/imagePool.h"
    "include/command.h"
    "include/qsgModel.h"
    "include/qsgBoard.h"
    "include/modbusMaster.h"
    "include/server.h"
    "include/client.h"

    "src/ssdp.cpp"
    "src/dllloader.cpp"
    "src/reaC++.cpp"
    "src/storage0.cpp"
    "src/util.cpp"
    "src/reaQML.cpp"
    "src/imagePool.cpp"
    "src/command.cpp"
    "src/qsgModel.cpp"
    "src/qsgBoard.cpp"
    "src/modbusMaster.cpp"
    "src/server.cpp"
    "src/client.cpp"
    )

# 创建动态库别名

#add_library(hello::library ALIAS REA)

target_link_libraries(REA PRIVATE Qt5::Core Qt5::Quick Qt5::Widgets Qt5::SerialPort Qt5::SerialBus)
# 包含指定头文件所在的目录
target_include_directories(REA PUBLIC  ${PROJECT_SOURCE_DIR}/include)
#target_include_directories(REA PUBLIC  ${PROJECT_SOURCE_DIR}/imageboardplugins)

# 创建可执行程序

#add_executable(hello_binary
#src/main.cpp
#)

# 链接动态库文件
#target_link_libraries( hello_binary PRIVATE hello::library)
