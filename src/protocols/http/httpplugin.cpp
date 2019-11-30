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

#include <cassert>

#include "http.h"
#include "url.h"
#include "utils.h"
#include "macro.h"
#include "httpplugin.h"
#include "debug.h"

typedef int error_code;
error_code get_proxy_host_null = -1;  //
error_code proxy_connect_failed = -2;  //
error_code get_url_failed = -3;  //
error_code direct_connect_failed = -4;  //
error_code http_get_failed = -5;  //
error_code parse_header_error = -6; //
error_code location_null = -7; //
error_code reset_url_error = -8; //
error_code bad_gateway_error = -9; //
error_code unkown_error = -10; //
error_code get_file_size_error = -11; //
error_code not_implemented_error = -12; //
error_code file_not_found_error = -13; // 404

using namespace std;

/* get file_size, set local_file_name, set timeout, setup debug log... */
int
HttpPlugin::get_info(Task *task)
{
    Http http;

    http.set_timeout(task->get_timeout());
    http.set_log(&debug_log);
    if(task->get_protocol() == HTTPS){
        http.set_use_ssl(true);
    }

    if(task->get_url_user() != NULL){
        http.auth(task->get_url_user(),
                  task->get_url_password() ? task->get_url_password() : "");
    }

    if(task->get_referer() != NULL){
        http.header("Referer", task->get_referer());
    }else{
        http.header("Referer", task->get_url());
    }

    if(task->get_file_size() > 0){
        // test the Range
        http.set_range(1);
    }

    if(task->get_proxy_type() == HTTP_PROXY){
        if(task->get_proxy_host() == NULL){
            return get_proxy_host_null;
        }
        if(http.connect(task->get_proxy_host(), task->get_proxy_port()) < 0){
            qCritical() << "http proxy connect failed" << endl;
            return proxy_connect_failed;
        }
        http.set_host(task->get_url_host(), task->get_url_port());
        if(task->get_proxy_user() != NULL){
            http.proxy_auth(task->get_proxy_user(),
                            task->get_proxy_password() ? task->get_proxy_password() : "");
        }
        if(http.get(task->get_url()) < 0){
            return get_url_failed;
        }
    }else{
        int rt = -33;
        if( (rt= http.connect(task->get_url_host(), task->get_url_port())) < 0){
            //    cerr << "http direct connect failed, rt:" << rt << endl;
            return direct_connect_failed;
        }

        if(http.get(task->get_encoded_path()) < 0){
            qCritical() << "http get failed" << endl;
            return http_get_failed;
        }
    }

    if(http.parse_header() < 0) return parse_header_error;

    int status_code = http.get_status_code();

    switch(status_code){
    case 200: // HTTP_STATUS_OK
    case 206: // HTTP_STATUS_PARTIAL_CONTENTS
    case 300: // HTTP_STATUS_MULTIPLE_CHOICES
    case 304: // HTTP_STATUS_NOT_MODIFIED
        break;
    case 301: // HTTP_STATUS_MOVED_PERMANENTLY
    case 302: // HTTP_STATUS_MOVED_TEMPORARILY
    case 303: // HTTP_SEE_OTHER
    case 307: // HTTP_STATUS_TEMPORARY_REDIRECT
    {// redirect
        task->set_file_size(-1); // if not, the new location's filesize is wrong
        const char *location = http.get_header("Location");
        if(location == NULL){
            // I do not know when this will happen, but no harm
            location = http.get_header("Content-Location");
            if(location == NULL) return location_null;
        }
        if(strcmp(location, task->get_url()) == 0) break;
        if(task->reset_url(location) < 0) return reset_url_error;
        return S_REDIRECT;
    }
    case 305: // HTTP_USE_PROXY
    {// get the content through the proxy
        task->set_file_size(-1); // if not, the new location's filesize is wrong
        return S_REDIRECT;
    }
    case 408: // HTTP_CLIENT_TIMEOUT
    case 504: // HTTP_GATEWAY_TIMEOUT
    case 503: // HTTP_UNAVAILABLE
    case 502: // HTTP_BAD_GATEWAY
    {// these errors can retry later
        return bad_gateway_error;
    }
    case 501: // Not Implemented
        return not_implemented_error;
    case 400: // HTTP BAD REQUEST
    {
        qCritical() << "http server returned bad request" << endl;
    }
    case 404:   // File Not found
    {
        return file_not_found_error;
    }
    default:
    {
        qDebug() << "Unkown: " <<  status_code << endl;
        return unkown_error;
    }
    }

    // if the page is an active page, we maybe can not get the filesize
    if(task->get_file_size() < 0){
        task->set_file_size( http.get_file_size());
        if(task->get_file_size() > 1){
            // we need test whether the Range header is supported or not
            return get_file_size_error;
        }
    }else{
        // IIS never return the Accept-Ranges header
        // We need check the Content-Range header for the resuming
        const char *ptr = http.get_header("Content-Range");
        if(ptr){
            while(*ptr != '\0' && !ISDIGIT(*ptr)) ptr ++;
            if(*ptr++ == '1' && *ptr == '-'){
                // get the filesize again for ensure the size
                task->set_file_size(1 + http.get_file_size());
                task->set_resumeSupported(true);
            }
        }
    }

    const char *filename;
    filename = http.get_header("Content-Disposition");
    if(filename){
        filename = strstr(filename, "filename=");
        if(filename){
            filename += strlen("filename=");
            if(task->get_local_file() == std::string("")){
                task->set_local_file(filename);
            }
        }
    }


    if(task->get_local_file() == std::string("") &&  task->get_file() == NULL ){
        task->set_local_file("index.html");
    }

    return 0;
}

int
HttpPlugin::download(Task& task, Block *block)
{
    block->state = STOP;
    if(task.get_resumeSupported()){
        if(block->downloaded >= block->size){
            block->state = EXIT;
            return 0;
        }else{
            block->bufferFile.seek(block->startPoint + block->downloaded);
        }
    }else{
        block->bufferFile.seek(0);
        block->downloaded = 0;
    }

    Http http;
    http.set_timeout(task.get_timeout());
    http.set_log(&debug_log);

    if(task.get_protocol() == HTTPS){
        http.set_use_ssl(true);
    }


    if(task.get_resumeSupported()){
        // the end is not set for the schedule purpose
        http.set_range(block->startPoint + block->downloaded);
    }

    if(task.get_url_user() != NULL){
        http.auth(task.get_url_user(),
                  task.get_url_password() ? task.get_url_password() : "");
    }

    if(task.get_referer() != NULL){
        http.header("Referer", task.get_referer());
    }else{
        http.header("Referer", task.get_url());
    }

    if(task.get_proxy_type() == HTTP_PROXY){
        if(http.connect(task.get_proxy_host(), task.get_proxy_port()) < 0){
            return -2;
        }
        http.set_host(task.get_url_host(), task.get_url_port());
        if(task.get_proxy_user() != NULL){
            http.proxy_auth(task.get_proxy_user(),
                            task.get_proxy_password() ? task.get_proxy_password() : "");
        }
        if(http.get(task.get_url()) < 0){
            return -2;
        }
    }else{
        if(http.connect(task.get_url_host(), task.get_url_port()) < 0){
            return -2;
        }
        if(http.get(task.get_encoded_path()) < 0){
            return -2;
        }
    }

    if(http.parse_header() < 0) return -2;
    // the service maybe unreachable temply
    // some servers alway return 302, so nasty
    if(http.get_status_code() >= 400) return -2;

_re_retr:
    block->state = RETR;
    if(block->bufferFile.retr_data_from(&http, &block->downloaded,
                                        block->size - block->downloaded) < 0){
        block->state = STOP;
        return -2;
    }

    if(task.get_resumeSupported() && block->downloaded < block->size){
        block->state = STOP;
        return -2;
    }

    block->state = WAIT;
    msleep(500);
    if(block->state == WAKEUP) goto _re_retr;
    block->state = EXIT;
    return 0;
};

int
HttpPlugin::get_dir_list(Task& task, const char *tempfile)
{
    return 0;
};
