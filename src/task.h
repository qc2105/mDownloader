/*  mDownloader - a multiple-threads downloading accelerator program that is based on Myget.
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

#ifndef _TASK_H
#define _TASK_H

#include <sys/types.h>
#include <QtGlobal>
#include <stdio.h>

#include <QtGlobal>

#include "macro.h"
#include "url.h"
#include "proxy.h"
#include <string>

class Task
{
	public:
		Task(void);
        ~Task(void);

	public:
        const qint64 get_file_size();
        void set_file_size(qint64 size);

        const std::string get_local_dir(void);
        void set_local_dir(const std::string& dir);

		const std::string get_local_file(void);
        void set_local_file(const std::string& file);

        const char* get_referer(void);


        const long get_retryInterval();


        const long get_timeout();


        const int get_ftpActive();
        void set_ftpActive(bool is_active);

        const int get_threadNum();
        void set_threadNum(int Num);

        const int get_tryCount();


        const bool get_isDirectory();
        void set_isDirectory(bool value);

        const bool get_resumeSupported();
        void set_resumeSupported(bool value);

        Protocol get_protocol() { return url.get_protocol();}

        const char* get_url() { return url.get_url(); }

        const char* get_file() { return url.get_file();}

        const char* get_dir() { return url.get_dir();}

        int reset_url(const char *url_origin) { return url.reset_url(url_origin);}

        int set_url(const char *url_origin) { return url.set_url(url_origin);}

        const char* get_url_host() { return url.get_host();}

        const char* get_url_user() { return url.get_user();}

        int get_url_port() { return url.get_port();}

        const char* get_url_password() { return url.get_password();}

        const char* get_encoded_path() { return url.get_encoded_path();}

        ProxyType get_proxy_type() { return proxy.get_type();}

        const char* get_proxy_host() { return proxy.get_host();}

        int get_proxy_port() { return proxy.get_port();}

        const char* get_proxy_user() { return proxy.get_user();}

        const char* get_proxy_password() { return proxy.get_password();}

        Task& operator = (Task& task);


    private:
        URL url;
        Proxy proxy;
        bool resumeSupported;
        bool isDirectory;
        int ftpActive;
        int threadNum;
        long timeout;
        long retryInterval;
        int tryCount;
        qint64 fileSize;
		std::string localDir;
		std::string localFile;
		char *referer;
};

#endif // _TASK_H
