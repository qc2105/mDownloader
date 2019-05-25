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


#ifndef _FTPPARSER_H
#define _FTPPARSER_H

#include <sys/types.h>
#include <QtGlobal>

#include <QtGlobal>

class FtpParser
{
	public:
		FtpParser();
		~FtpParser();

		const char *get_file(void);
		const char *get_link(void);
		qint64 get_size(void);
		time_t get_time(void);
		char get_type(void);

		int parse(const char *line);

	private:
		void reset(void);
		int process_dos(char *line);
		int process_unix(char *line);

	private:
		const char *file;
		const char *link;
		qint64 size;
		time_t time;
		char type;
};

#endif // _FTPPARSER_H
