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

#ifndef _PROXY_H
#define _PROXY_H

#include "url.h"

enum ProxyType
{
	HTTP_PROXY,
	FTP_PROXY,
	SOCKS5_PROXY,
	NONE_PROXY
};

class Proxy
{	
	public:
		Proxy();
		~Proxy();

		ProxyType get_type(void);
        void set_type(ProxyType type);

        const char* get_host(void);
        void set_host(const char *host);

        int get_port(void);
        void set_port(int port);

        const char* get_user(void);
        void set_user(const char *user);

        const char* get_password(void);
		void set_password(const char *password);

		Proxy& operator = (Proxy &proxy);

	private:
		ProxyType type;
		const char *host;
		int port;
		const char *user;
		const char *password;
};

#endif // _PROXY_H
