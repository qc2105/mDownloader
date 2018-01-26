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

#ifndef _PROGRESSBAR_H
#define _PROGRESSBAR_H

#include <QtGlobal>

#include <sys/types.h>
#include <QtGlobal>

#define MINIMUM_SCREEN_WIDTH 45 // the minimum screen width 
#define DEFAULT_SCREEN_WIDTH 80 // the default screen width when can not get the screen width
#define MAXIMUM_SCREEN_WIDTH 256 //the maximum screen width
#define DIRTY_WIDTH 35

class ProgressBar
{
	public:
		ProgressBar(qint64 total_size = 0, int block_num = 1);
		~ProgressBar();

		void set_start_point(qint64 *start_point);
        void set_total_size(qint64 size){ totalSize = size; }
		void set_block_num(int num);
		void update(qint64 *data);
		void init(void);
        qint64 get_curr_downloaded(void);
        long get_percent(void);
        char *get_downloaded(void);
        char *get_downloadRate(void);
        char *get_eta(void);

        qint64 *data;

	private:
        static void screen_width_change(int signo);
		
	private:
		double lastTime; // the last time of update
		qint64 lastDownloaded; // the total size of the already download part
		qint64 totalSize; // the total size of the file in bytes
		qint64 *startPoint; // save the start points
		int blockNum; // blockNum
		bool show; // can show or not

		float rates[12];
		int rateIndex;
		int rateCount;

        qint64 curr_downloaded;
        long percent;
        char downloaded[5];
        char downloadRate[5];
		char eta[6];
		char graph[MAXIMUM_SCREEN_WIDTH - DIRTY_WIDTH + 1];
		static int graphWidth;
		int direction;
};

#endif // _PROGRESSBAR_H
