cmake_minimum_required(VERSION 3.1.0)

project("mDownloader")

if (UNIX)
    set(CMAKE_CXX_FLAGS -Wall)
endif (UNIX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5Sql CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)

set(mDownloader_SRCS
    src/main.cpp
    src/advio.cpp 
    src/block.cpp 
    src/debug.cpp 
    src/downloader.cpp 
    src/ftp.cpp 
    src/ftpparser.cpp 
    src/ftpplugin.cpp 
    src/header.cpp 
    src/http.cpp 
    src/httpplugin.cpp 
    src/plugin.cpp 
    src/progressbar.cpp 
    src/proxy.cpp 
    src/task.cpp 
    src/url.cpp 
    src/utils.cpp 
    src/status.cpp 
    src/ui/newtask.cpp 
    src/ui/mainwindow.cpp 
    src/ui/jobview.cc 
    src/ui/errormessagebox.cpp 
    src/dbmanager.cpp
    src/ui/newtask.ui
    src/mDownloader_zh_CN.ts
    src/mDownloader.qrc
)

qt5_add_resources(src/mDownloader.qrc)
add_executable(mDownloader ${mDownloader_SRCS})
target_link_libraries(mDownloader Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::Sql)
