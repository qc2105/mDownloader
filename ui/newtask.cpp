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

#include <stdlib.h>
#include <stdio.h>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QUuid>
#include <QUrl>


#include "newtask.h"
#include "ui_newtask.h"


NewTask::NewTask(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewTask)
{
    ui->setupUi(this);

    m_dir = QDir::toNativeSeparators(QDir::homePath());
    m_tempFilePath = m_dir;
    ui->lineEditUrl->setText(tr("Paste or input the URL of the file you want to download here."));
    ui->lineEditSaveLocation->setText(m_dir + QDir::separator() + ui->lineEditFileName->text());
    ui->spinBoxThreadNum->setMinimum(1);
    ui->spinBoxThreadNum->setValue(1);
    connect(ui->lineEditUrl, SIGNAL(textChanged(QString)), this, SLOT(setFileNameSlot(QString)));
    connect(ui->lineEditFileName, SIGNAL(textChanged(QString)), this, SLOT(setFileNameSlot(QString)));
}

NewTask::~NewTask()
{
    delete ui;
}

void NewTask::showMyself(void)
{
    show();
}

void NewTask::on_buttonBoxWhetherOk_accepted()
{
    if (QUrl(ui->lineEditTempFilePath->text()).isValid())
    {
        emit resumeJob(ui->lineEditTempFilePath->text());
    }
    else if (QUrl(ui->lineEditUrl->text()).isValid() && QUrl(ui->lineEditFileName->text()).isValid())
    {
        emit setDownloadedDirectory(m_dir);
        emit setFileName(ui->lineEditFileName->text());
        emit setSaveLocation(ui->lineEditSaveLocation->text());
        emit setThreadNum(ui->spinBoxThreadNum->value());
        emit runDownloader(ui->lineEditUrl->text());
        emit newJob(ui->lineEditFileName->text(), m_dir, ui->lineEditUrl->text(), ui->spinBoxThreadNum->value());
    }
}

void NewTask::setFileNameSlot(QString Url)
{
    m_localFileName = Url.section("/", -1);
    if (m_localFileName.isEmpty()) {
        m_localFileName = "index.html";
    }
    ui->lineEditFileName->setText(/*"file-" + QUuid::createUuid().toString()+ */m_localFileName);
    ui->lineEditSaveLocation->setText(QDir::toNativeSeparators(m_dir + QDir::separator() +  /*"file-" + QUuid::createUuid().toString()+*/ m_localFileName));
}

void NewTask::on_pushButtonSetSaveLocation_clicked()
{
    m_dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QDir::homePath(), QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    ui->lineEditSaveLocation->setText(QDir::toNativeSeparators(m_dir + QDir::separator() + ui->lineEditFileName->text()));
}

void NewTask::on_pushButtonSetTempFile_clicked()
{
    m_tempFilePath = QFileDialog::getOpenFileName(this, tr("Open mg! Temp File"), QDir::homePath(), tr("TempFile (*.mg!)"), Q_NULLPTR, QFileDialog::DontResolveSymlinks);
    ui->lineEditTempFilePath->setText(QDir::toNativeSeparators(m_tempFilePath));
}
