/*  mDownloader - a multiple-threads downloading accelerator program that is based on Myget.
 *  
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

#include <cstring>
#include <cassert>
#include <QDebug>

#include "http.h"
#include "utils.h"
#include "macro.h"
#include "header.h"


/***************************************************
 * class Http implement
 * the relative RFCS can be found at
 * http://www.w3.org/Protocols/
 ***************************************************/
#define USER_AGENT "mDownloader"
#define HTTP_VERSION "1.1"


Http::Http()
{
    qSock = new QSslSocket();

	timeout = 30;

	useSSL = false;

	log = &default_log;
	request.set_attr("Connection", "close");
	request.set_attr("User-Agent", USER_AGENT);
	//request.set_attr("Range", "bytes=0-"); error when the fiesize is zero
	request.set_attr("Accept", "*/*");
}

Http::~Http()
{
    qSock->close();
    delete qSock;
}

void
Http::set_timeout(long timeout)
{
	this->timeout = timeout;
}

void
Http::set_log(void(*log)(const char *, ...))
{
	this->log = log;
}


void
Http::set_use_ssl(bool use)
{
	this->useSSL = use;
}


int
Http::connect(const char *host, int port)
{
    if (useSSL)
    {
        qSock->setPeerVerifyMode(QSslSocket::VerifyNone);
        qSock->connectToHostEncrypted(QString(host), port);
        if (!qSock->waitForEncrypted(timeout * 1000))
        {
            return -1;
        }
    }
    else
    {
        qSock->connectToHost(QString(host), port);
        if (!qSock->waitForConnected(timeout * 1000))
        {
            return -1;
        }
    }

    remoteAddr = qSock->peerAddress();

    set_host(host, port);

	return 0;
}

int 
Http::header(const char *attrName, const char *attrValue)
{
	return request.set_attr(attrName, attrValue);
}

#if WIN32
#define snprintf sprintf_s
#endif

int 
Http::auth(const char *user, const char *password)
{
	const char *base64;

	snprintf(buf, 1024, "%s:%s", user, password);
	base64 = base64_encode(buf);
	snprintf(buf, 1024, "Basic %s", base64);
	delete[] base64;

	return request.set_attr("Authorization", buf);
}

int
Http::proxy_auth(const char *user, const char *password)
{
	const char *base64;

	snprintf(buf, 1024, "%s:%s", user, password);
	base64 = base64_encode(buf);
	snprintf(buf, 1024, "Basic %s", base64);
	delete[] base64;

	return request.set_attr("Proxy-Authorization", buf);
}

int
Http::set_range(qint64 start, qint64 end)
{

	if(end < 0){
        snprintf(buf, 1024, "bytes=%lld-", start);
	}else{
        snprintf(buf, 1024, "bytes=%lld-%lld", start, end);
	}

	return request.set_attr("Range", buf);
}

int
Http::set_host(const char *host, int port)
{
	if(strchr(host, ':')){ /* ipv6 */
		if((useSSL && port != 443) || (!useSSL && port != 80)){
			snprintf(buf, 1024, "[%s]:%d", host, port);
		}else{
			snprintf(buf, 1024, "[%s]", host);
		}
	}else{
		if((useSSL && port != 443) || (!useSSL && port != 80)){
			snprintf(buf, 1024, "%s:%d", host, port);
		}else{
			snprintf(buf, 1024, "%s", host);
		}
	}

	return request.set_attr("Host", buf);
}

int
Http::send_head()
{
	HeadDataNode *it;
	int ret;
    if (qSock->state() == QAbstractSocket::ConnectedState)
    {
        qDebug() << "Send_head() qSock connected" << endl;
    }
    else
    {
        return -1;
    }

	for(it = request.head; it != NULL; it = it->next){
		snprintf(buf, 1024, "%s: %s\r\n", it->attrName, it->attrValue);
        if((ret=qSock->write(buf)) < 0) return ret;
        qSock->waitForBytesWritten(timeout * 1000);
		log("%s: %s\r\n", it->attrName, it->attrValue);
	}

	snprintf(buf, 1024, "\r\n");
    if((ret=qSock->write(buf)) < 0) return ret;
    qSock->waitForBytesWritten(timeout * 1000);
	log("\r\n");

	return 0;
}

int
Http::head(const char *url)
{
	int ret;

	snprintf(buf, 1024, "HEAD %s HTTP/%s\r\n", url, HTTP_VERSION);

    if((ret=qSock->write(buf)) < 0) return ret;
    qSock->waitForBytesWritten(timeout * 1000);
	log("HEAD %s HTTP/%s\r\n", url, HTTP_VERSION);

	RETURN_IF_FAILED(send_head());

	return 0;
}

int
Http::get(const char *url)
{
	int ret;

    snprintf(buf, 1024, "GET %s HTTP/%s\r\n", url, HTTP_VERSION);
    if((ret=qSock->write(buf)) < 0) return ret;
    qSock->waitForBytesWritten(timeout * 1000);
	log("GET %s HTTP/%s\r\n", url, HTTP_VERSION);

    RETURN_IF_FAILED(send_head());

	return 0;
}

// not implement
#if 0
int
Http::post(const char *url)
{
}
#endif

/* the normal head just like this 
HTTP/1.1 200 OK
Date: Tue, 03 May 2005 07:37:36 GMT
Server: Apache/2.0.52 (Gentoo/Linux) PHP/4.3.10
X-Powered-By: PHP/4.3.10
Content-Length: 57
Connection: close
Content-Type: text/html
*/

int
Http::parse_header()
{
	int ret;
	char *ptr, *attrName, *attrValue;

	response.remove_all();

    qSock->waitForReadyRead(timeout * 1000);
    ret = qSock->readLine(buf, 1024);
	if(ret < 0) return ret;
	if(ret == 0) return -1;
	log("%s", buf);

	ptr = buf;
	while(*ptr == ' ') ptr ++;
	// skip the http version info
	while(*ptr != '\0' && *ptr != ' ' && *ptr != '\r' && *ptr != '\n') ptr ++;
	if(*ptr != ' ') return -1;
	while(*ptr == ' ') ptr ++;
	statusCode = atoi(ptr);

	while(1){
        qSock->waitForReadyRead(timeout * 1000);
        ret = qSock->readLine(buf, 1024);
		if(ret < 0) return ret;
		if(ret == 0) return -1;
		log("%s", buf);

		ptr = buf;
		while(*ptr == ' ') ptr ++;
		attrName = ptr;
		if(*ptr == '\r' || *ptr == '\n') break;
		while(*ptr != '\0' && *ptr != ':' && *ptr != '\r' && *ptr != '\n') ptr ++;
		if(*ptr == ':'){
			*ptr = '\0';
			ptr ++;
		}else{
			return -1;
		}

		while(*ptr == ' ') ptr ++;
		attrValue = ptr;
		while(*ptr != '\0' && *ptr != '\r' && *ptr != '\n') ptr ++;
		*ptr = '\0';

		response.set_attr(attrName, attrValue);
	} // end of while

	// get fileSize in bytes
	ptr = (char*)response.get_attr("Content-Length");
	if(ptr != NULL){
		for(fileSize = 0; *ptr >= '0' && *ptr <= '9'; ptr ++){
			fileSize = fileSize * 10 + (*ptr - '0');
		}
	}else{
		fileSize = -1;
	}

	// get Transfer-Encoding
	ptr = (char*)response.get_attr("Transfer-Encoding");
    if(ptr != NULL && strcmp(ptr, "chunked") == 0){
		isChunked = true;
		chunkedSize = 0;
	}else if(fileSize >= 0){
		isChunked = false;
		chunkedSize = fileSize;
	}else{ // some http proxyer dose not return the content-size when the file is large
		isChunked = false;
		chunkedSize = -1;
	}

	return 0;
} // end of parse_header;

const char*
Http::get_header(const char *attr_name)
{
	return response.get_attr(attr_name);
}

qint64
Http::get_file_size()
{
	return fileSize;
}

int
Http::get_status_code()
{
	return statusCode;
}

bool
Http::accept_ranges()
{
	if(response.get_attr("Accept-Ranges") != NULL
            && strcmp(response.get_attr("Accept-Ranges"), "none") != 0){
		return true;
	}

	return false;
}

int
Http::read_data(char *buffer, int maxsize)
{
	int ret;

	if(chunkedSize == 0){
		if(!isChunked){
			return 0; // EOF
		}else{ // get chunked size in hex
			int size;
			char *ptr;
_read_data_again:
            qSock->waitForReadyRead(timeout * 1000);
            ret = qSock->readLine(buf, 1024);
			if(ret < 0) return ret;
			if(ret == 0) return -1;
			ptr = buf;
			while(*ptr == ' ') ptr ++ ;
			if(*ptr == '\r' || *ptr == '\n') goto _read_data_again;
			sscanf(buf, "%x", &size);
			chunkedSize = size;
		}
	}

	if(chunkedSize == 0) return 0; // EOF

	if(chunkedSize > 0){
		maxsize = maxsize < chunkedSize ? maxsize : (int)chunkedSize;
	}
    qSock->waitForReadyRead(timeout * 1000);
    ret = qSock->read(buffer, maxsize);

	if(chunkedSize > 0){
		if(ret <= 0) return -1; // Can not get all of the data
		chunkedSize -= ret; // chunkedSize decrease
	}else{
		if(ret <= 0) return ret; // error or EOF
	}

	return ret;
}

int
Http::data_ends()
{ // not very reasonable
	return chunkedSize == 0 ? 0 : -1;
}
