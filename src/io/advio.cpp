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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <cstring>

#include "advio.h"
#include "macro.h"
#include "plugin.h"


/****************************************************
 * class PluginIO implement
 ****************************************************/
PluginIO::PluginIO()
{
	// do nothing
}

PluginIO::~PluginIO()
{
	// do nothing
}

int
PluginIO::read_data(char *buffer, int maxsize)
{
	// this function is a virtual function and must
	// be overloaded by the base class of the plugin
	return -1;
};

/****************************************************
 * class BufferFile implement
 ****************************************************/

BufferFile::BufferFile()
{
	ptr = buf;
	left = FILE_BUFFER_SIZE;
}

BufferFile::~BufferFile()
{
    flush();
    diskFile.close();
}

bool BufferFile::open(const char *file)
{
	ptr = buf;
	left = FILE_BUFFER_SIZE;
    diskFile.setFileName(file);

    return diskFile.open(QIODevice::ReadWrite);
}

void
BufferFile::close()
{
    flush();
    diskFile.close();
}

// flush the data to the hard-disk, and return the bytes
// on success, return -1 on error
int
BufferFile::flush()
{
	int count;
	int wc;
	char *pptr;
	int bc;

	count = FILE_BUFFER_SIZE - left;
	bc = count;
	pptr = buf;
	while(count > 0){
_flush_again:
        wc = diskFile.write(pptr, count);
		if(wc < 0){
			if(errno == EINTR) goto _flush_again;
			perror("write back to file failed");
			break;
		}
		pptr += wc;
		count -= wc;

	}

	ptr = buf;
	if(count > 0){ // some error happend
		memmove(buf, pptr, count);
		left += bc - count;
		return -1;
	}else{
		left = FILE_BUFFER_SIZE;
		return bc;
	}
};

qint64
BufferFile::seek(qint64 off_set)
{
	// before seek we must flush the buffer,
	// if not we will write the data to the wrong postion
    flush();
    if (off_set == 0)
    {
        return 0;
    }
    return diskFile.seek(off_set);
}

int
BufferFile::truncate(qint64 length)
{
    return diskFile.resize(length);
}

// get length bytes data from pio, and argument rtlength
// is used to real-time reflect the change, if the length is set
// to -1, this function will read all the data
qint64
BufferFile::retr_data_from(PluginIO *pio, qint64 *rtlength, qint64 length)
{
	assert(pio != NULL);
	int ret;
	qint64 data_count = 0;

	while(1){
		ret = pio->read_data(ptr, left);
		if(ret == 0){
			break;
		}else if(ret < 0){
			return ret;
		}
		left -= ret;
		ptr += ret;
        *rtlength += ret;
		data_count += ret;
		if(left <= MIN_BUFFER_LEFT){
            flush();
		}
		if(length >= 0 && data_count >= length) break;
	}

	return data_count;
};
