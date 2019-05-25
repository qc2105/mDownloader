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

#include "ftpplugin.h"
#include "ftp.h"
#include "macro.h"
#include "utils.h"
#include "ftpparser.h"
#include "debug.h"
#include "header.h"

using namespace std;

#if WIN32
#define snprintf sprintf_s
#endif

int
FtpPlugin::get_info(Task *task)
{
	Ftp ftp;
	int ret;

    ftp.set_timeout(task->get_timeout());
	ftp.set_log(&debug_log);
    if(ftp.connect(task->get_url_host(), task->get_url_port())< 0){
		return -2;
	}

    ret = ftp.login(task->get_url_user(), task->get_url_password());
	if(ret < 0){
		return -2;
	}else if(ret > 0){
        sleep(task->get_retryInterval());
		return -2;
	}

	// test REST
	ret = ftp.rest(100);
	if(ret < 0){
		return -2;
	}else if(ret == 0){
        task->set_resumeSupported(true);
	}

	// passive or port
	int port;
    ret = ftp.pasv(&port);
    if(ret < 0){
        return -2;
    }else if(ret > 0){
        task->set_ftpActive(true);
    }

	// can access
    ret = ftp.cwd(task->get_dir());
	if(ret < 0){
		return -2;
	}else if(ret > 0){
		return -1;
	}

	// test a directory or not
    ret = ftp.cwd(task->get_file());
	if(ret < 0){
		return -2;
	}else if(ret == 0){
        task->set_isDirectory(true);
        if(task->get_file() != NULL ){
			char *ptr;
            ptr = new char[strlen(task->get_file()) + 2];
            sprintf(ptr, "%s/", task->get_file());
            task->reset_url(ptr);
			delete[] ptr;
		}
	}

    if(task->get_isDirectory()) return 0;

    qint64 size = 0;
    ret = ftp.size(task->get_file(), &size);
    task->set_file_size(size);

	if(ret < 0){
		return -2;
	}else if(ret > 0){
		return -1;
	}

	return 0;
};

int
FtpPlugin::download(Task &task, Block *block)
{
	Ftp ftp;
	int ret;

	block->state = STOP;
    if(task.get_resumeSupported()){
		if(block->downloaded >= block->size){
			block->state = EXIT;
			return 0;
		}else{
			block->bufferFile.seek(block->startPoint + block->downloaded);
		}
	}else{
		block->downloaded = 0;
		block->bufferFile.seek(0);
	}

    ftp.set_timeout(task.get_timeout());
	ftp.set_log(&debug_log);
    ftp.set_mode(task.get_ftpActive());

    if(ftp.connect(task.get_url_host(), task.get_url_port()) < 0){
		return -2;
	}

    ret = ftp.login(task.get_url_user(), task.get_url_password());
	if(ret < 0){
		return -2;
	}else if(ret > 0){
        sleep(task.get_retryInterval());
		return -2;
	}

    ret = ftp.cwd(task.get_dir());
	if(ret < 0){
		return -2;
	}else if(ret > 0){
		return -1; // may be the dir can not be accessed
	}

	if(ftp.type("I") < 0) return -2;

    ret = ftp.retr(task.get_file(),
			block->startPoint + block->downloaded);
	if(ret < 0){
		return -2;
	}else if(ret > 0){
		return -1; // can not access this file
	}

_re_retr:
	block->state = RETR;
	if(block->bufferFile.retr_data_from(&ftp, &block->downloaded,
				block->size - block->downloaded) < 0){
		block->state = STOP;
		return -2;
	}

    if(task.get_resumeSupported() && block->downloaded < block->size){
		block->state = STOP;
		return -2;
	}

	// wait the server to schedule
	block->state = WAIT;
    msleep(500);
	if(block->state == WAKEUP) goto _re_retr;
	block->state = EXIT;
	return 0;
};

int
FtpPlugin::relogin(Ftp *ftp, Task &task)
{
	int ret;
	ret = ftp->reconnect();
	if(ret < 0) return ret;

    ret = ftp->login(task.get_url_user(), task.get_url_password());
	if(ret < 0){
		return ret;
	}else if(ret > 0){
        sleep(task.get_retryInterval());
		return -1;
	}

	return 0;
};

int
FtpPlugin::recursive_get_dir_list(Task &task, Ftp *ftp, const char *tempfile, 
		const char *absdir, FILE *rfd, FILE *wfd, qint64 *woff)
{
	char currdir[1024];
	char file[1024];
	const char *ptr;
	int ret;
	FtpParser fp;
	qint64 filesize;

	while(1){
		// get the current work directory
		if(*woff == 0){
            snprintf(currdir, 1024, "%s%s",
                    task.get_dir() ? "/" : "",
                    task.get_dir() ? task.get_dir() : "");
		}else{
			while(1){
				if(fread(&filesize, sizeof(qint64), 1, rfd) != 1) return 0;
				if(fgets(currdir, 1024, rfd) == NULL){
					return 0;
				}else if(currdir[0] == '/'){
					currdir[strlen(currdir) - 1] = '\0';
					break;
				}
			}
		}
	
		fseek(wfd, *woff, SEEK_SET);
		// change work directory
		ret = ftp->cwd(absdir);
		if(ret < 0){
			return ret;
		}
		if(currdir[0] != '\0'){
			ret = ftp->cwd(currdir + 1);
			if(ret < 0){ // timeout or other system error
				return ret;
			}else if(ret > 0){ // the permission problem, just ignore
				continue;
			}
		}
		// get directory list
		ret = ftp->list();
		if(ret < 0){
			return ret;
		}else if(ret > 0){
			// list directory content is not allowed
			return E_LIST_DIR;
		}
		while(1){
			ret = ftp->gets_data(file, 1024);
			if(ret == 0 && ftp->data_ends() == 0) break;
			if(ret < 0){ 
				return -1;
			}
			if(fp.parse(file) < 0) continue;
			ptr = fp.get_file();
			if(ptr == NULL) continue;
			if(strcmp(ptr, ".") == 0 || strcmp(ptr, "..") == 0) continue;
			if(fp.get_type() == 'd'){
				filesize = 0;
				fwrite(&filesize, sizeof(qint64), 1, wfd);
				fprintf(wfd, "%s/%s\n", currdir, ptr);
			}else{
				filesize = fp.get_size();
				fwrite(&filesize, sizeof(qint64), 1, wfd);
				fprintf(wfd, "%s/%s\n", currdir + 1, ptr);
			}
		}
		fflush(wfd);
		*woff = ftell(wfd);
	}

	return 0;
};

int
FtpPlugin::get_dir_list(Task& task, const char *tempfile)
{

	assert(tempfile != NULL);
	// dir1/dir2/dir3	file
	// /dir1/dir2/dir3	dir
	Ftp ftp;
	int ret;
	bool first_conn = true;
	char *absdir = NULL;
	
	ftp.set_log(&debug_log);
    ftp.set_timeout(task.get_timeout());
    ftp.set_mode(task.get_ftpActive());
	while(1){
        if(ftp.connect(task.get_url_host(), task.get_url_port()) == 0){
			break;
		}
	}

_ftp_get_dir_list_conn:

	if(first_conn){
		first_conn = false;
	}else{
		if(ftp.reconnect() < 0) goto _ftp_get_dir_list_conn;
	}
    ret = ftp.login(task.get_url_user(), task.get_url_password());
	if(ret < 0){
		goto _ftp_get_dir_list_conn;
	}else if(ret > 0){
        sleep(task.get_retryInterval());
		goto _ftp_get_dir_list_conn;
	}
	// get absolute directory
	if(ftp.pwd(&absdir) != 0) goto _ftp_get_dir_list_conn;

	FILE *rfd;
	FILE *wfd;
	qint64 woff;

	rfd = fopen(tempfile, "r");
	wfd = fopen(tempfile, "r+");
	woff = 0;

	while(1){
		ret = recursive_get_dir_list(task, &ftp, tempfile, absdir, rfd, wfd, &woff);
		if(ret != 0){
			if(ret == E_LIST_DIR){
				fclose(wfd);
				fclose(rfd);
				delete[] absdir;
				return -1;
			}
		}else{
			break;
		}
		while(relogin(&ftp, task) < 0);
	}

	fclose(wfd);
	fclose(rfd);
	delete[] absdir;
	return 0;
};
