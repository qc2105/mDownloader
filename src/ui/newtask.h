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

#ifndef NEWTASK_H
#define NEWTASK_H

#include <QDialog>
#include <QProgressDialog>


namespace Ui {
class NewTask;
}

class NewTask : public QDialog
{
    Q_OBJECT

public:
    explicit NewTask(QWidget *parent = 0);
    ~NewTask();

private slots:
    void showMyself(void);
    void on_buttonBoxWhetherOk_accepted();
    void on_pushButtonSetSaveLocation_clicked();
    void setFileNameSlot(QString);
    void on_pushButtonSetTempFile_clicked();

private:
    Ui::NewTask *ui;
    QString m_dir;
    QString m_localFileName;
    QString m_tempFilePath;


signals:
    void newJob(QString fileName, QString DownloadedDirectory, QString URL, int threadNum);
    void resumeJob(QString tempFilePath);
    void runDownloader(QString);
    void setFileName(QString);
    void setSaveLocation(QString);
    void setDownloadedDirectory(QString);
    void setThreadNum(int);

};

#endif // NEWTASK_H
