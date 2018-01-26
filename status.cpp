/****************************************************************************************
** status.cpp is part of yoDownet
**
** Copyright 2012, 2013 Alireza Savand <alireza.savand@gmail.com>
**
** Imported to mDownloader and Modified by Richard Qin<qc2105@qq.com> in 2015.
**
** yoDownet is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** yoDownet is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************************/

#include "status.h"

Status::Status(QObject *parent) :
    QObject(parent)
{
}


QString Status::downloadModeString() const
{
    switch(_dlMode){
    case NewDownload:
        return tr("New Download");
        break;
    case ResumeDownload:
        return tr("Resume Download");
        break;
    }
    return QString();
}


void Status::setDownloadMode(const Status::DownloadMode dlMode)
{
    _dlMode = dlMode;
}

Status::DownloadMode Status::downloadMode() const
{
    return _dlMode;
}

void Status::setDownloadStatus(const Status::DownloadStatus dlStatus)
{
    _dlStatus = dlStatus;
}

Status::DownloadStatus Status::downloadStatus() const
{
    return _dlStatus;
}
