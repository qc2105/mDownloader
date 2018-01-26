/*  mDownloader - a multiple-threads downloading accelerator program that is based on Myget.
 *  Homepage: http://qinchuan.me/article.php?id=100
 *  2015 By Richard (qc2105@qq.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Myget - A download accelerator for GNU/Linux
 *  Homepage: http://myget.sf.net
 *  2005 by xiaosuo
 */


#ifndef _FTP_H
#define _FTP_H

#include <sys/types.h>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>

#define INET6_ADDRSTRLEN 65

#include "macro.h"
#include "advio.h"

/***************************************************
 * class Ftp declaration
 * this class only contain basic ftp commands wraped
 * *************************************************/


class Ftp
    : public PluginIO
{
	public:
		Ftp();
		~Ftp();

		int connect(const char *addrstr, int port);
		int reconnect();

		// the commands return 0 if successfully
		// active or passive, default is passive
		int set_mode(int type);
		// set log function
		void set_log( void(*f)(const char*, ...) );
		// set timeout for connection , read and write
		void set_timeout(long timeout);
		// login with user and password, if user is
		// NULL, anonymous will be used as user
		int login(const char *user=NULL, const char *password=NULL);
		// change work dir
		int cwd(const char *dir);
		// print current dir, the returned value must be freed
		int pwd(char** dir);
		// get the size of file
		int size(const char *file, qint64 *size);
		// set offset from the begin, useful when n threads download
		int rest(qint64 offset);
		// get the dir list, if dir is NULL, list the current dir
		int list(const char *dir=NULL);
		// get the file: the file must be in the current dir
		int retr(const char *file, qint64 offset = -1);
		// set the data-type:A(ASCII) or I(binary)
		int type(const char *type);
		// get the data from dataConn
		int read_data(char *buf, int maxsize);
		// read a line from the dataConn
		int gets_data(char *buf, int maxsize);
		// get all the data?
		int data_ends();
		// quit
		int quit();

        int port_cmd(int port);
		int pasv(int *port);

	private:
		Ftp(Ftp&);
		void operator = (Ftp);

		int ftp_cmd(const char *cmd=NULL, const char *args=NULL);
		int ftp_data_cmd(const char *cmd, const char *args, qint64 offset = -1);
		void (*log)(const char *, ...);
        static void default_log(const char *, ...){};

	protected:
        QTcpSocket *qCtrLSock;
        QTcpSocket *qDataSock;
        QTcpServer *qDataServer;

        QHostAddress qRemoteAddr;
        QHostAddress qLocalAddr;
        int m_port;

		char stateLine[1024];
		long timeout;
		int mode; //active or passive
};

#endif // _FTP_H
