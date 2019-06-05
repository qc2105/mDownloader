cmake_minimum_required(VERSION 3.1.0)

project("mDownloader_tests")

if (UNIX)
    set(CMAKE_CXX_FLAGS -Wall)
endif (UNIX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(../src/db/ ../src/protocols/ ../src/)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5Sql CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

set(tests_SRCS
    db_manager/test_db_manager.cpp
    ../src/db/dbmanager.cpp
    Header/test_header.cpp 
    ../src/protocols/header.cpp 
    ../src/utils.cpp
)

add_executable(tests ${tests_SRCS})
target_link_libraries(tests Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::Sql gtest pthread)

