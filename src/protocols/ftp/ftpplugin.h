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


#ifndef _FTPPLUGIN_H
#define _FTPPLUGIN_H

#include "plugin.h"
#include "task.h"
#include "ftp.h"
#include <QThread>

class FtpPlugin
    : public Plugin, public QThread
{
	public:
		int get_info(Task *task);
		int download(Task &task, Block *block);
		int get_dir_list(Task &task, const char *tempfile);
		int relogin(Ftp *ftp, Task &task);
		int recursive_get_dir_list(Task &task, Ftp *ftp, const char *tempfile,
				const char *absdir, FILE *rfd, FILE *wfd, qint64 *woff);
};

#endif // _FTPPLUGIN_H
