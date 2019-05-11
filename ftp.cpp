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

#include <cassert>
#include <sys/types.h>
#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

#include "ftp.h"
#include "advio.h"
#include "utils.h"
#include "debug.h"

#if WIN32
#define snprintf sprintf_s
#endif

/********************************************
 * Ftp implement
 * ******************************************/

Ftp::Ftp()
{
    qCtrLSock = new QTcpSocket(0);
    qDataSock = new QTcpSocket(0);
    qDataServer = new QTcpServer(0);

    timeout = 30;
    mode = PASV;
	stateLine[0] = 0;
	log = &default_log;
}

Ftp::~Ftp()
{
    qCtrLSock->close();
    qDataSock->close();
    delete qCtrLSock;
    delete qDataSock;
    delete qDataServer;
}

int
Ftp::connect(const char *addrstr, int port)
{
	log(_("Resolve address...\n"));
	log(_("Connecting...\n"));

    qCtrLSock->connectToHost(QString(addrstr), port);
    if (!qCtrLSock->waitForConnected(timeout * 1000))
    {
        return -1;
    }

    qRemoteAddr = qCtrLSock->peerAddress();
    qLocalAddr = qCtrLSock->localAddress();
    m_port = port;

	return 0;
}

int
Ftp::reconnect()
{
	log(_("Reconnecting...\n"));

    qCtrLSock->close(); qDataSock->close();

    qCtrLSock->connectToHost(qRemoteAddr, m_port);
    if (!qCtrLSock->waitForConnected(timeout * 1000))
    {
        return -1;
    }

	return 0;
}

int
Ftp::ftp_cmd(const char* cmd, const char* args)
{
    int ret;
    char buffer[1024];

	if(cmd){
		snprintf(buffer, 1024, "%s%s%s\r\n", cmd, 
				args == NULL ? "" : " " ,
				args == NULL ? "" : args);
		log("%s", buffer);
        if ((ret = qCtrLSock->write(buffer)) < 0)
            return ret;
        qCtrLSock->waitForBytesWritten(timeout *1000);
	}

	do{
        qCtrLSock->waitForReadyRead(timeout * 1000);
        ret = qCtrLSock->readLine(buffer, 1024);
		if(ret < 0){
			return ret;
		}else if(ret == 0){
			return -1;
		}
		log("%s", buffer);
	}while(buffer[3] != ' ' || !ISDIGIT(buffer[0]));

	strcpy(stateLine, buffer + 4);

	return atoi(buffer);
}

int
Ftp::login(const char* user, const char* password)
{
	int ret;

	// get all the welcome message
	ret = ftp_cmd();
	switch(ret){
		case 220:
			break;
		default:
			return ret;
	}

	ret = ftp_cmd("USER", user == NULL ? "anonymous" : user);
	switch(ret){
		case 230:
			return 0;
		case 331:
			break;
		default:
			return ret;
	}

	ret = ftp_cmd("PASS", password == NULL ? "myget@myget" : password);
	switch(ret){
		case 230:
			return 0;
		default:
			return ret;
	}
};

int
Ftp::cwd(const char* dir)
{
	int ret;

	ret = ftp_cmd("CWD", dir ? dir : "/");
	switch(ret){
		case 250:
			return 0;
		default:
			return ret;
	}
};

int
Ftp::pwd(char** dir)
{
	int ret;
	char *ptr;
	char *tdir;
	
	ret = ftp_cmd("PWD");
	switch(ret){
		case 257:
			break;
		default:
			return ret;
	}

	for(ptr = stateLine; *ptr != '\0' && *ptr != '"'; ptr ++) ;
	if(*ptr != '"') return -1;
	tdir = ptr + 1;
	for(ptr ++; *ptr != '\0' && *ptr != '"'; ptr ++) ;
	if(*ptr != '"') return -1;
	*ptr = '\0';
	*dir = StrDup(tdir);

	return 0;
};

int
Ftp::size(const char *file, qint64 *size)
{
	int ret;
	char *ptr;
	
	if(!file) return -1;

	ret = ftp_cmd("SIZE", file);
	switch(ret){
		case 213:
			break;
		default:
			return ret;
	}

	*size = 0;
	ptr = stateLine;
	while(*ptr == ' ') ptr++;
	while(ISDIGIT(*ptr)){
		*size *= 10;
		*size += *ptr -= '0';
		ptr ++;
	}
	
	return 0;
};

int
Ftp::rest(qint64 offset)
{
	int ret;
	if(offset < 0) return -1;
	char buffer[64];

    snprintf(buffer, 64, "%lld", offset);
	ret = ftp_cmd("REST", buffer);
	switch(ret){
		case 350:
			return 0;
		default:
			return ret;
	}
};

int
Ftp::set_mode(int mode)
{
	if(mode != PASV && mode != PORT) return -1;
	this->mode = mode;
	return 0;
}

void
Ftp::set_log( void(*f)(const char*str, ...))
{
	this->log = f;
}

void
Ftp::set_timeout(long timeout)
{
	this->timeout = timeout;
}

int
Ftp::type(const char* type)
{
	int ret;

	if(!type) return -1;
	ret = ftp_cmd("TYPE", type);
	switch(ret){
		case 200:
			return 0;
		default:
			return ret;
	}
}

int
Ftp::pasv(int *port)
{
	int ret;
	char *ptr;
	*port = 0;

	/**********************************
	 * RFC2428
	 * ipv4:
	 * <=PASV 
	 * =>227 xxxx(h1,h2,h3,h4,p1,p2)
	 * ipv6:
	 * normal:
	 * <=EPSV
	 * extension:
	 * <=EPSV 1  //use ipv4 connection
	 * <=EPSV 2  //use ipv6 connection
	 * =>229 xxxx(|||6446|)
	 */

    if(qRemoteAddr.protocol() == QAbstractSocket::IPv4Protocol){ // ipv4
		ret = ftp_cmd("PASV");
		switch(ret){
			case 227:
				break;
			default:
				return ret;
		}
		// xxxx(h1,h2,h3,h4,p1,p2)
		if((ptr=strrchr(stateLine, ',')) == NULL) return -1;
		*port = atoi(ptr+1);
		do{
			ptr -- ;
		}while(ptr != stateLine && ISDIGIT(*ptr));
		if(ptr == stateLine) return -1;
		*port += atoi(ptr+1)<<8;
    }else if(qRemoteAddr.protocol() == QAbstractSocket::IPv6Protocol){
		ret = ftp_cmd("EPSV", "2");
		switch(ret){
			case 229:
				break;
			default:
				return ret;
		}
		// xxxx(|||6446|)
		if((ptr=strrchr(stateLine, '|')) == NULL) return -1;
		do{
			ptr --;
		}while(ptr != stateLine && ISDIGIT(*ptr));
		if(ptr == stateLine) return -1;
		*port = atoi(ptr+1);
	}else
		return -1;

	return 0;
};

int
Ftp::port_cmd(int port)
{
	int ret;
	char buffer[128];
	char addr[INET6_ADDRSTRLEN];
    char *ptr = NULL;

	/************************************
	 * RFC2428
	 * ipv4:
	 * <=PORT 10,20,12,66,port>>8,port&0xff
	 * <=EPRT |1|132.235.1.2|6275|
	 * ipv6: 
	 * <=EPRT |2|IPv6.ascii|PORT.ascii|
	 * =>200 xxx
	 */
    if(qLocalAddr.protocol() == QAbstractSocket::IPv4Protocol){
        strcpy(addr, qLocalAddr.toString().toStdString().c_str());
        ptr = addr;
		while(*ptr){
			if(*ptr == '.') *ptr = ',';
			ptr ++;
		}
		snprintf(buffer, 128, "%s,%d,%d", addr, port>>8, port&0xff);
		ret = ftp_cmd("PORT", buffer);
    }else if(qLocalAddr.protocol() == QAbstractSocket::IPv6Protocol){
        strcpy(addr, qLocalAddr.toString().toStdString().c_str());
		snprintf(buffer, 128, "|2|%s|%d|", addr, port);
		ret = ftp_cmd("EPRT", buffer);
	}else
		return -1;

	switch(ret){
		case 200:
			break;
		default:
			return ret;
	}

	return 0;
}

// set command to the server and wating data-connection
// opened successfully
int
Ftp::ftp_data_cmd(const char* cmd, const char* args, qint64 offset)
{
	int ret;
	int port;

	if(mode == PASV){ //passive
		ret = pasv(&port);
		if(ret != 0) return ret;

        qDataSock->connectToHost(qRemoteAddr, port);
        if (!qDataSock->waitForConnected(timeout * 1000))
        {
            return -1;
        }
    }else if(mode == PORT){ //active
        qDataServer->listen(qLocalAddr);
        ret = port_cmd(qDataServer->serverPort());
        if(ret != 0) return ret;
	}

	if(offset > 0){
		ret = rest(offset);
		if(ret != 0) return ret;
	}
	// send command and wait
	ret = ftp_cmd(cmd, args);
    switch(ret){
		case 150:
		case 125:
			break;
		default:
			return ret;
	}

	if(mode == PORT){
        qDataServer->waitForNewConnection(-1);
        qDataSock = qDataServer->nextPendingConnection();
	}

    return 0;
}

// quit normally
int
Ftp::quit()
{
	int ret;

	ret = ftp_cmd("QUIT");
	switch(ret){
		case 221:
			break;
		default:
			return ret;
	}
	return 0;
}

// get maxsize bytes, will be used by RETR
int
Ftp::read_data(char *buffer, int maxsize)
{
    qDataSock->waitForReadyRead(timeout * 1000);
    return qDataSock->read(buffer, maxsize);
}

// get a line, will be used by LIST
int
Ftp::gets_data(char *buffer, int maxsize)
{
    qDataSock->waitForReadyRead(timeout * 1000);
    return qDataSock->readLine(buffer, maxsize);
}

int
Ftp::data_ends()
{
	int ret;
    qDataSock->close();
	ret = ftp_cmd();
	switch(ret){
		case 226:
		case 250:
			return 0;
		default:
			return ret;
	}
}

int
Ftp::list(const char *dir)
{
	return ftp_data_cmd("LIST", dir);
}

int
Ftp::retr(const char *file, qint64 offset)
{
	if(!file) return -1;

	return ftp_data_cmd("RETR", file, offset);
}
