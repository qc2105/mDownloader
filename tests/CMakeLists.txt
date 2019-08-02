cmake_minimum_required(VERSION 3.1.0)

project("mDownloader_tests")

if (UNIX)
    set(CMAKE_CXX_FLAGS -Wall)
endif (UNIX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(../src/db/ ../src/protocols/ ../src/ ../src/protocols/http ../src/protocols/ftp ../src/io)

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

set(test_Downloader_SRCS
    ../src/io/advio.cpp 
    ../src/io/block.cpp 
    ../src/debug.cpp 
    ../src/downloader.cpp 
    ../src/protocols/ftp/ftp.cpp 
    ../src/protocols/ftp/ftpparser.cpp 
    ../src/protocols/ftp/ftpplugin.cpp 
    ../src/protocols/header.cpp 
    ../src/protocols/http/http.cpp 
    ../src/protocols/http/httpplugin.cpp 
    ../src/protocols/plugin.cpp 
    ../src/progressbar.cpp 
    ../src/protocols/proxy.cpp 
    ../src/task.cpp 
    ../src/protocols/url.cpp 
    ../src/utils.cpp 
    ../src/status.cpp 
    ../src/db/dbmanager.cpp
    TestDownloader/test_Downloader.cpp 
)

add_executable(testServer TINY_WebServer/csapp.c TINY_WebServer/tiny.c)
target_link_libraries(testServer pthread)

add_executable(tests ${tests_SRCS})
target_link_libraries(tests Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::Sql gtest pthread)

add_executable(testDownloader ${test_Downloader_SRCS})
target_link_libraries(testDownloader Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::Sql gtest pthread)

