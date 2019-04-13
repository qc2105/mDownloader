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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMessageBox>

#include <QProgressDialog>
#include "../downloader.h"
#include "errormessagebox.h"
#include "jobview.h"

QT_BEGIN_NAMESPACE
class QAction;
class QCloseEvent;
class QLabel;
class QProgressDialog;
class QSlider;
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSize sizeHint() const Q_DECL_OVERRIDE;
    const Downloader *downloaderForRow(int row) const;

public slots:
    void on_error_happens(QString);
    void set_newJobFileName(QString);
    void set_newJobDownloadedDirectory(QString);
    void addJob(QString fileName, QString DownDir, QString URL, int threadNUM);
    void resuMeJob(QString tempFilePath);

private slots:
    virtual void closeEvent(QCloseEvent *);

    void about();
    void setActionsEnabled();


    // First version multi-jobs UI
    bool addJob();
    void resuMeJob();
    void removeJob();
    void pauseJob();
    void openDir();
    void moveJobUp();
    void moveJobDown();
    void updateState(QString state);
    void updateProgress(int percent);
    void updateDownloadRate(QString speed);
    void updateDownloaded(QString downloaded);
    void updateRemainingTime(QString remainingTime);

signals:
    void newTaskShow(void);
    void m_quit(void);
    void resumeTask(void);

private:
    qint64 speedBytesPerSecond;
    bool m_has_error_happend;
    ErrorMessageBox m_eMsgBox;
    QMessageBox msgBox;

    QProgressDialog *quitDialog;

    bool saveChanges;
    QString lastDirectory;

    JobView *jobView;
    QAction *pauseJobAction;
    QAction *removeJobAction;
    QAction *openDirAction;
    QAction *resuMeJobAction;

    struct Job {
        QString fileName;
        QString tempFilePath;
        QString destinationDir;
        QString url;
        int threadNum;
        Downloader *downloader;
    };

    QString newJobFileName;
    QString newJobDestinationDirectory;

    QList<Job> jobs;
    int jobsStopped;
    int jobsToStop;

    int rowOfDownloader(Downloader *dloader) const;
};

#endif // MAINWINDOW_H
