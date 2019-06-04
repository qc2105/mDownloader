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




/*some advance IO classes and functions*/

#ifndef _ADVIO_H
#define _ADVIO_H

//#define MAXLONG 2147483647 // 2G
#define BUFSIZE 1024

#include <cassert>
#include <fcntl.h>

#include "macro.h"
#include <sys/types.h>


#include <QFile>


// this class must be inherited by the base class of the 
// plugin and the read_data function must be overloaded
class PluginIO
{
public:
    PluginIO();
    ~PluginIO();

    virtual int read_data(char *buffer, int maxsize);
};

class BufferFile
{
public:
    BufferFile();
    ~BufferFile();

    bool open(const char *file);
    void close();

    qint64 seek(qint64 off_set);
    qint64 retr_data_from(PluginIO *pio, qint64 *rtlength, qint64 length = -1);

private:
    char buf[FILE_BUFFER_SIZE];
    char *ptr;
    int left;
    int truncate(qint64 length);
    int flush();
    QFile diskFile;
};

#endif // _ADVIO_H
