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


#ifndef _UTILS_H
#define _UTILS_H

#include <QtGlobal>
#include <sys/types.h>
#include <QtGlobal>

#define MIN(a, b) ((a) < (b)) ? (a) : (b)
// for international
#define _(x) (x)

#define ISDIGIT(x) ((x) >= '0' && (x) <= '9')

#define ISBLANK(x) ((x) == ' ' || (x) == '\t')

#define RETURN_IF_FAILED( test ) \
	{ int ret; if((ret=(test)) != SUCC) return ret; }
#define EXIT_IF_FAILED( test ) \
	{ int ret; if((ret=(test)) != SUCC) return ret; }

// the same as strdup, but use new to get the memory 
char* StrDup(const char *str);

// the returned value must be freed
const char* base64_encode(const char *str, int length = -1);
const char* base64_decode(const char *str, int length = -1);

// try to get the screen width
int determine_screen_width(void);
// get the current time in secondes
double get_current_time(void);
// conver size to 333M, 111K, 1G
void convert_size(char *sizeStr, qint64 size);
// convert time to 11d23 11h12 12:34
void convert_time(char *timeStr, double time);

// file exist
bool file_exist(const char *filename);

enum class ShutdownDialogAction
{
    Exit,
    Shutdown,
    Suspend,
    Hibernate
};

void shutdownComputer(const ShutdownDialogAction& action);

#endif // _UTILS_H
