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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sys/types.h>
#include <signal.h>

#include <QDebug>

#include "progressbar.h"
#include "utils.h"

/*
 * percent : graph : already download : download rate : remain time
 * 23% [=============>          =============>          ===>        ] [900M] [800K/s] [ETA:02:54]
 */

#define RIGHT 1
#define LEFT 0

int ProgressBar::graphWidth = 0;

ProgressBar::ProgressBar(qint64 total_size, int block_num)
    :totalSize(total_size), blockNum(block_num), lastDownloaded(0), direction(RIGHT), data(NULL)
{
	char *term;
	int i;
	int screenWidth;

	rateIndex = 0;
    memset(downloaded,0x0, sizeof(downloaded));
    memset(downloadRate, 0x0, sizeof(downloadRate));
    sprintf(eta, "--:--");
    rateCount = 0;
	for(i = 0; i < 12; i++ ){
		rates[i] = 0;
	}

	startPoint = new qint64[blockNum];	
	for(i = 0; i < blockNum; i++ ){
		startPoint[i] = 0;
	}
	lastTime = get_current_time();

    show=true;

	screenWidth = determine_screen_width();
	if(screenWidth == 0){
		screenWidth = DEFAULT_SCREEN_WIDTH;
	}else{
		screenWidth = screenWidth < MINIMUM_SCREEN_WIDTH ?
			MINIMUM_SCREEN_WIDTH : screenWidth;
		screenWidth = screenWidth > MAXIMUM_SCREEN_WIDTH ?
			MAXIMUM_SCREEN_WIDTH : screenWidth;
        //signal(SIGWINCH, &screen_width_change);
	}
	graphWidth = screenWidth - DIRTY_WIDTH;
};

ProgressBar::~ProgressBar()
{
	delete[] startPoint;
};

void
ProgressBar::screen_width_change(int signo)
{
	int screenWidth;

	screenWidth = determine_screen_width();
	if(screenWidth == 0){
		screenWidth = DEFAULT_SCREEN_WIDTH;
	}else{
		screenWidth = screenWidth < MINIMUM_SCREEN_WIDTH ?
			MINIMUM_SCREEN_WIDTH : screenWidth;
		screenWidth = screenWidth > MAXIMUM_SCREEN_WIDTH ?
			MAXIMUM_SCREEN_WIDTH : screenWidth;
	}
	graphWidth = screenWidth - DIRTY_WIDTH;
};

void
ProgressBar::init(void)
{
	int i;

	lastTime = get_current_time();
	lastDownloaded = 0;
	rateIndex = 0;
	rateCount = 0;
	for(i = 0; i < 12; i++ ){
		rates[i] = 0;
	}
}

qint64
ProgressBar::get_curr_downloaded(void)
{
    return curr_downloaded;
}

long
ProgressBar::get_percent(void)
{
    return percent;
}

char *
ProgressBar::get_downloaded(void)
{
    return downloaded;
}

char *
ProgressBar::get_downloadRate(void)
{
    return downloadRate;
}

char *
ProgressBar::get_eta(void)
{
    return eta;
}

void
ProgressBar::set_block_num(int num)
{
	int i;

	blockNum = num;
	delete[] startPoint;
	startPoint = new qint64[blockNum];
	for(i = 0; i < blockNum; i++ ){
		startPoint[i] = 0;
	}
}

void
ProgressBar::set_start_point(qint64 *data)
{
	int i;
	
	assert(data != NULL);
	for(i = 0; i < blockNum; i ++){
		startPoint[i] = data[i];
	}
}

// data [block1][block2][block3]
void
ProgressBar::update(qint64 *data)
{
	double curr_time;
	float rate;
	int i;
	qint64 graph_step;

	assert(data != NULL);

	// avoid too often update
	curr_time = get_current_time();
	if(curr_time - lastTime < 0.25) return;

	curr_downloaded = 0;
	for(i = 0; i < blockNum; i ++){
		curr_downloaded += data[i];
	}

	if(totalSize > 0 && lastDownloaded >= totalSize ) return;

	convert_size(downloaded, curr_downloaded);
	rate = 0;
	if(lastDownloaded > 0){
		// we need more reasonable rate, how to implement
		rates[rateIndex] = (curr_downloaded - lastDownloaded ) / (curr_time - lastTime);
		if(rateCount < 12) rateCount ++;
		rateIndex ++;
		rateIndex %= 12;
		for(i = 0; i < 12; i ++){
			rate += rates[i];
		}
		rate /= rateCount;
	}
	convert_size(downloadRate, (qint64)rate);
	graph_step = totalSize / graphWidth;
	if(graph_step <= 0){
		percent = 100;
		sprintf(eta, "--:--");
		int pos = 0;
		// if can not get the filesize , show user something else
		// [            <==>                  ]
		for(i = 0; i < graphWidth; i ++){
			if(graph[i] == '<') pos = i;
			graph[i] = ' ';
		}

		if(direction == RIGHT && pos >= graphWidth - 4){
			direction = LEFT;
		}else if(direction == LEFT && pos <= 0){
			direction = RIGHT;
		}

		if(direction == RIGHT){
			pos ++;
		}else{
			pos --;
		}

		graph[pos++] = '<';
		graph[pos++] = '=';
		graph[pos++] = '=';
		graph[pos++] = '>';
		graph[graphWidth] = '\0';
	}else{
		// [=============>          =============>          ===>        ]
		if(rate >= 1){
			convert_time(eta, (totalSize - curr_downloaded) / rate);
		}else{
			sprintf(eta, "--:--");
		}

        percent = (int)((float)curr_downloaded / (float)totalSize * 100);

		percent = MIN(percent, 100);


		for(i = 0; i < graphWidth; i++) graph[i] = ' ';

		int j, k;
		int num;

		for(j = 0; j < blockNum; j ++){
			// download one by one
			if(data[j] <= 0) continue;
			i = (int)(startPoint[j] / graph_step);
			num = (int)((startPoint[j] + data[j]) / graph_step);
			num = num < graphWidth ? num : graphWidth;
			for(; i < num; i ++){
				graph[i] = '=';
			}

			for(k = j + 1; k < blockNum; k ++){
				if(startPoint[k] != -1) break;
			}
			if(k == blockNum){
				if(startPoint[j] + data[j] >= totalSize) continue; 
			}else{
				if(startPoint[j] + data[j] >= startPoint[k]) continue;
			}
			if(num > 0) graph[num - 1] = '>';
		}

		graph[graphWidth] = '\0';
	}

    qDebug() << percent << graph << downloaded << downloadRate << eta << '\n';

	lastTime = curr_time;
	lastDownloaded = curr_downloaded;
};
