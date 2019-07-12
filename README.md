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

All tests are under folder tests. 

# Latest official build / deb package for deepin 15.10.1 and Ubuntu 18.04 LTS
	https://github.com/qc2105/mDownloader/releases/latest
	a) sudo apt-get install qt5-default
	b) wget https://github.com/qc2105/mDownloader/releases/download/v1.1/mdownloader_1.1-1_amd64.deb
	c) sudo dpkg -i mdownloader_1.1-1_amd64.deb
	After doing these, just type mDownloader in your terminal to run the app.
	The executable's full path is /usr/bin/mDownloader.

# Screenshots
![screenshot1](https://raw.githubusercontent.com/qc2105/mDownloader/master/Screenshot%20from%202019-04-13%2016-53-31.png)
