# mDownloader
mDownloader: A cross-platform multiple-threads GUI download accelerator based on Qt and modified mytget(https://github.com/lytsing/mytget).
It is fast, with maximum 99 threads do the downloading job at the same time, takes full advantage of your network resources when does HTTP/HTTPS/FTP downloading. 

# How to build

## Under Windows:

### 1. Tools that you will need

	a) Visual Studio Community 2013, download from http://www.visualstudio.com/en-us/news/vs2013-community-vs.aspx or google this: vs2013.4_ce_enu.iso;
	b) Qt 5.4.0 for Windows 32-bit (VS 2013), download from http://download.qt-project.org/official_releases/qt/5.4/5.4.0/qt-opensource-windows-x86-msvc2013-5.4.0.exe;
	c) CMake (Version >= 3.1.0)

### 2. Build

	a) Run cmake-gui
	b) Follow the instructions

## Under deepin-15.10.1 / Ubuntu 14.04 and above:

 	a) sudo apt-get install qt5-default libsqlite3-dev
	b) Install CMake (Version >= 3.1.0)
	c) mkdir build; cd build 
	d) cmake ..
	e) make -j12
	f) sudo make install
	g) mDownloader

# Automated Testing

All tests are under folder test. You'd better run the tests under Windows 7, because some wierd random errors happen when I run them under Ubuntu 14.04.

# Latest official build 
N/A

# Screenshots
![screenshot1](https://raw.githubusercontent.com/qc2105/mDownloader/master/Screenshot%20from%202019-04-13%2016-53-31.png)
