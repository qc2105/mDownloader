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

#include "task.h"
#include "utils.h"

Task::Task()
{
	fileSize = -1;
	isDirectory = false;
	resumeSupported = false;
	tryCount = 99;
	retryInterval = 5;
	timeout = 30;
    ftpActive = PASV;
    threadNum = 10;
	localDir = NULL;
	localFile = NULL;
	referer = NULL;
}

Task::~Task()
{
    delete[] localDir;
	delete[] localFile;
	delete[] referer;
}
	
const qint64 Task::get_file_size()
{
    return fileSize;
}
void Task::set_file_size(qint64 size)
{
    fileSize = size;
}

const char*
Task::get_local_dir(void)
{
	return localDir;
}

void
Task::set_local_dir(const char *dir)
{
    delete[] localDir;
    localDir = StrDup(dir);
}


const char*
Task::get_local_file(void)
{
	return localFile;
}

void
Task::set_local_file(const char *file)
{
    delete[] localFile;
    localFile = StrDup(file);
}

const char*
Task::get_referer(void)
{
	return referer;
}

const long
Task::get_retryInterval()
{
    return retryInterval;
}

const long
Task::get_timeout()
{
    return timeout;
}

const int
Task::get_ftpActive()
{
    return ftpActive;
}

void
Task::set_ftpActive(bool is_active)
{
    if (is_active)
    {
        ftpActive = PORT;
    }
    else
    {
        ftpActive = PASV;
    }
}

const int
Task::get_threadNum()
{
    return threadNum;
}

void
Task::set_threadNum(int Num)
{
    threadNum = Num;
}

const int Task::get_tryCount()
{
    return tryCount;
}

//void Task::set_tryCount(int count)
//{
//    tryCount = count;
//}

const bool
Task::get_isDirectory()
{
    return isDirectory;
}

void
Task::set_isDirectory(bool value)
{
    isDirectory = value;
}

const bool
Task::get_resumeSupported()
{
    return resumeSupported;
}

void
Task::set_resumeSupported(bool value)
{
    resumeSupported = value;
}

Task&
Task::operator = (Task& task)
{
    if(this == &task) return *this;

    delete[] localDir;
    delete[] localFile;
    delete[] referer;
    localDir = StrDup(task.get_local_dir());
    localFile = StrDup(task.get_local_file());
    referer = StrDup(task.get_referer());
    fileSize = task.fileSize;
    isDirectory = task.isDirectory;
    resumeSupported = task.resumeSupported;
    tryCount = task.tryCount;
    retryInterval = task.retryInterval;
    timeout = task.timeout;
    ftpActive = task.ftpActive;
    threadNum = task.threadNum;
    url = task.url;
    proxy = task.proxy;

    return *this;
}

