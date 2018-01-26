/****************************************************************************************
** status.h is part of yoDownet
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

#ifndef STATUS_H
#define STATUS_H

#include <QObject>
#include <QTime>
#include <QUuid>

class Status : public QObject
{
    Q_OBJECT
    Q_ENUMS(DownloadMode)
    Q_ENUMS(DownloadStatus)

public:
    explicit Status(QObject *parent = 0);

    enum DownloadMode{
        NewDownload,
        ResumeDownload
    };

    // Change Downloader::setState() if you change this.
    enum DownloadStatus{
        Idle,
        Starting,
        Downloading,
        Finished,
        Failed,
        Paused,
        Pausing,
        Stopping
    };

    QString downloadModeString() const;

    void setDownloadMode(const DownloadMode dlMode);
    DownloadMode downloadMode() const;

    void setDownloadStatus(const DownloadStatus dlStatus);
    DownloadStatus downloadStatus() const;

private:
    DownloadMode _dlMode;
    DownloadStatus _dlStatus;

};

#endif // STATUS_H
