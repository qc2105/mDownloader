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

#ifndef _BLOCK_H
#define _BLOCK_H

#include "advio.h"
#include <QThread>

enum ThreadState
{
	STOP,
	RETR,
	WAIT,
	WAKEUP,
	EXIT,
	JOINED
};

class Block
{
	public:
		Block();

	public:
        QThread *ptr_thread;
		ThreadState state;
		BufferFile bufferFile;
		qint64 startPoint;
		qint64 downloaded;
		qint64 size;
};

#endif // _BLOCK_H
