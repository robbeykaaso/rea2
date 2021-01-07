cmake_minimum_required(VERSION 3.1)

project(REA8 LANGUAGES CXX)

add_definitions(-DEXPORTDLL)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets Core Quick SerialPort SerialBus REQUIRED)

set(my_Sources
    "main.cpp" #WIN32: to disable the console window
    "qml.qrc"
    "reaC++.h"
    "reaC++.cpp"
    "reaQML.h"
    "reaQML.cpp"
    "util.h"
    "util.cpp"
    "tess.h"
    "qsgModel.h"
    "qsgModel.cpp"
    "imagePool.h"
    "imagePool.cpp"
    "qsgBoard.h"
    "qsgBoard.cpp"

    "dllloader.cpp"
    "command.h"
    "command.cpp"
    "storage0.h"
    "storage0.cpp"
    "ssdp.h"
    "ssdp.cpp"
    "server.h"
    "server.cpp"
    "client.h"
    "client.cpp"
    "modbusMaster.h"
    "modbusMaster.cpp"
    "unitTest.cpp"
    )

set(my_Libs
    Qt5::Core Qt5::Quick Qt5::Widgets Qt5::SerialPort Qt5::SerialBus
    ${REA_LIBS}
    dbghelp
    )

add_executable(${PROJECT_NAME}
    ${my_Sources}
    )
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${my_Libs}
    )

include_directories(${REA_INCLUDE_DIRS})
