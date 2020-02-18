cmake_minimum_required(VERSION 3.1.0)

project("mDownloader")

if (UNIX)
    set(CMAKE_CXX_FLAGS -Wall)
endif (UNIX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(src/protocols src/db src/protocols/http src/protocols/ftp src/ src/io src/ui)

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
    src/io/advio.cpp 
    src/io/block.cpp 
    src/debug.cpp 
    src/downloader.cpp 
    src/protocols/ftp/ftp.cpp 
    src/protocols/ftp/ftpparser.cpp 
    src/protocols/ftp/ftpplugin.cpp 
    src/protocols/header.cpp 
    src/protocols/http/http.cpp 
    src/protocols/http/httpplugin.cpp 
    src/protocols/plugin.cpp 
    src/progressbar.cpp 
    src/protocols/proxy.cpp 
    src/task.cpp 
    src/protocols/url.cpp 
    src/utils.cpp 
    src/status.cpp 
    src/ui/newtask.cpp 
    src/ui/mainwindow.cpp 
    src/ui/jobview.cc 
    src/ui/errormessagebox.cpp
    src/ui/newtask.ui
    src/mDownloader.qrc
    src/db/dbmanager.cpp
    src/translations/mDownloader_zh_CN.ts
)

qt5_add_resources(src/mDownloader.qrc)

if (UNIX)
    add_executable(mDownloader ${mDownloader_SRCS})
elseif (WIN32)
    add_executable(mDownloader WIN32 ${mDownloader_SRCS})
endif (UNIX)

target_link_libraries(mDownloader Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network Qt5::Sql)

install(TARGETS mDownloader
	RUNTIME DESTINATION /usr/bin)


