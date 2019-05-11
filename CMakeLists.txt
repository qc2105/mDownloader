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
    main.cpp
    advio.cpp 
    block.cpp 
    debug.cpp 
    downloader.cpp 
    ftp.cpp 
    ftpparser.cpp 
    ftpplugin.cpp 
    header.cpp 
    http.cpp 
    httpplugin.cpp 
    plugin.cpp 
    progressbar.cpp 
    proxy.cpp 
    task.cpp 
    url.cpp 
    utils.cpp 
    status.cpp 
    ui/newtask.cpp 
    ui/mainwindow.cpp 
    ui/jobview.cc 
    ui/errormessagebox.cpp 
    dbmanager.cpp
    ui/newtask.ui
    mDownloader_zh_CN.ts
    mDownloader.qrc
)

qt5_add_resources(mDownloader.qrc)
add_executable(mDownloader ${mDownloader_SRCS})
target_link_libraries(mDownloader Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::Sql)
