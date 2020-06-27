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

#include <stdio.h>
#include <cassert>
#include <cstring>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "utils.h"

#include <QDateTime>
#include <QFile>

using namespace std;

char*
StrDup(const char *str)
{
	char *ptr;
	int i;

	if(str == NULL) return NULL;

	for(i = 0; str[i] != '\0'; i ++) ;
	ptr = new char[i + 1];
	for(i = 0; str[i] != '\0'; i ++){
		ptr[i] = str[i];
	}
	ptr[i] = '\0';

	return ptr;
};

/* the relative RFC: RFC 3548 */
const char* 
base64_encode(const char *str, int length)
{  
	static const char base64_table[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
  };

	int str_enc_len;
	char *str_enc;
	unsigned char *ptr, *pptr;
	int i;
	int length_mod;

	length = (length < 0 ? strlen(str) : length);

	length_mod = length % 3;
	length /= 3;
	str_enc_len = length * 4;
	str_enc_len += ((length_mod == 0) ? 0 : 4);

	str_enc = new char[str_enc_len + 1];

	ptr = (unsigned char*)str;
	pptr = (unsigned char*)str_enc;
	for(i = 0; i < length; i ++){
		*pptr++ = base64_table[(ptr[0] >> 2)];
		*pptr++ = base64_table[((ptr[0] & 0x3) << 4) + (ptr[1] >> 4)];
		*pptr++ = base64_table[((ptr[1] & 0xf) << 2) + (ptr[2] >> 6)];
		*pptr++ = base64_table[(ptr[2] & 0x3f)];
		ptr += 3;
	}

	if(length_mod == 1){
		*pptr++ = base64_table[(ptr[0] >> 2)];
		*pptr++ = base64_table[((ptr[0] & 0x3) << 4)];
		*pptr++ = '=';
		*pptr++ = '=';
	}else if(length_mod == 2){
		*pptr++ = base64_table[(ptr[0] >> 2)];
		*pptr++ = base64_table[((ptr[0] & 0x3) << 4) + (ptr[1] >> 4)];
		*pptr++ = base64_table[((ptr[1] & 0xf) << 2)];
		*pptr++ = '=';
	}

	*pptr = '\0';

	return str_enc;
};

/* the decode function is not useful in this software so no need to write it
   static const char* 
   base64_decode(const char *str, int longth = -1)
   {
   };
   */

/* Determine the width of the terminal we're running on.  If that's
 * not possible, return 0.  */
int
determine_screen_width(void)
{
#ifndef TIOCGWINSZ
	return 0;
#else  /* TIOCGWINSZ */
	struct winsize wsz;

	if(ioctl(fileno(stderr), TIOCGWINSZ, &wsz) < 0) return 0;

	return wsz.ws_col;
#endif /* TIOCGWINSZ */
}


double
get_current_time(void)
{
    QDateTime dt;

    return dt.currentDateTime().toTime_t();
}

// conver size to 333M, 111K, 1G
void
convert_size(char *sizeStr, qint64 size)
{
	double dsize = size;

	if(dsize < 0){
        sprintf(sizeStr, "%3dB", 0);
		return;
	}

	if(dsize < 1000){
		sprintf(sizeStr, "%3ldB", (long)dsize);
		return;
	}
	dsize /= 1024;
	if(dsize < 1000){
		if(dsize <= 9.9){
			sprintf(sizeStr, "%.1fK", dsize);
		}else{
			sprintf(sizeStr, "%3ldK", (long)dsize);
		}
		return;
	}
	dsize /= 1024;
	if(dsize < 1000){
		if(dsize <= 9.9){
			sprintf(sizeStr, "%.1fM", dsize);
		}else{
			sprintf(sizeStr, "%3ldM", (long)dsize);
		}
		return;
	}
	dsize /= 1024;
	if(dsize < 1000){
		if(dsize <= 9.9){
			sprintf(sizeStr, "%.1fG", dsize);
		}else{
			sprintf(sizeStr, "%3ldG", (long)dsize);
		}
		return;
	}
};
// convert time to 11d23 11h12 12:34
void
convert_time(char *timeStr, double time)
{
	long sec, min, hour, day;
	
	min = (long)time / 60; // min
	sec = (long)time % 60; // sec

	if(min < 60){
        sprintf(timeStr, "%02ld:%02ld", min, sec);
		return;
	}

	hour = min / 60;
	min %= 60;

	if(hour < 24){
        sprintf(timeStr, "%2ldh%2ld", hour, min);
		return;
	}

	day = hour / 24;
	hour %= 24;

	if(day < 100){
        sprintf(timeStr, "%2ldd%2ld", day, hour);
		return;
	}
	
	sprintf(timeStr, "--:--");
};

bool
file_exist(const char *file)
{
	assert(file != NULL);

    QFile qf(file);

    return qf.exists();
}

#include <windows.h>
#include <powrprof.h>
#include <Shlobj.h>
#include <memory>

void shutdownComputer(const ShutdownDialogAction& action)
{
#ifdef WIN32
	HANDLE hToken;            // handle to process token
	TOKEN_PRIVILEGES tkp;     // pointer to token structure
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return;
	// Get the LUID for shutdown privilege.
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
		&tkp.Privileges[0].Luid);

	tkp.PrivilegeCount = 1; // one privilege to set
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// Get shutdown privilege for this process.

	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
		(PTOKEN_PRIVILEGES)NULL, 0);

	// Cannot test the return value of AdjustTokenPrivileges.

	if (GetLastError() != ERROR_SUCCESS)
		return;

	if (action == ShutdownDialogAction::Suspend) {
		::SetSuspendState(false, false, false);
	}
	else if (action == ShutdownDialogAction::Hibernate) {
		::SetSuspendState(true, false, false);
	}
	else {
		const QString msg = QString("mDownloader will shutdown the computer now because all downloads are complete.");
		auto msgWchar = std::make_unique<wchar_t[]>(static_cast<size_t>(msg.length()) + 1);
		msg.toWCharArray(msgWchar.get());
		::InitiateSystemShutdownW(nullptr, msgWchar.get(), 10, true, false);
	}

	// Disable shutdown privilege.
	tkp.Privileges[0].Attributes = 0;
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
#endif
}
