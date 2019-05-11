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

#include <QProcess>
#include <QStringList>
#include <stdlib.h>
#include <QChar>
#include <QDir>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>

#include "../status.h"
#include "../utils.h"
#include "newtask.h"

#include "mainwindow.h"

extern void
catch_ctrl_c(int signo);

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          quitDialog(0), saveChanges(false),
                                          newJobFileName(""), newJobDestinationDirectory("")
{
    m_has_error_happend = false;
    db.createTable();
    QMetaObject::invokeMethod(this, "loadSettings", Qt::QueuedConnection);
    setWindowIcon(QIcon(":/ui/icons/motocool.jpg"));

    // Initiallize headers
    QStringList headers;
    headers << tr("Name") << tr("Downloaded/Total") << tr("Progress") << tr("Speed")
            << tr("Status") << tr("Remaining time");

    // Main job list
    jobView = new JobView(this);
    jobView->setHeaderLabels(headers);
    jobView->setSelectionBehavior(QAbstractItemView::SelectRows);
    jobView->setAlternatingRowColors(true);
    jobView->setRootIsDecorated(false);
    setCentralWidget(jobView);

    // Set header resize modes and initial section sizes
    QFontMetrics fm = fontMetrics();
    QHeaderView *header = jobView->header();
    header->resizeSection(0, fm.width("typical-name-length-for-a-download-task"));
    header->resizeSection(1, fm.width(headers.at(1) + "100MB/999MB"));
    header->resizeSection(2, fm.width(headers.at(2) + "100%"));
    header->resizeSection(3, qMax(fm.width(headers.at(3) + "   "), fm.width(" 1023.0 KB/s")));
    header->resizeSection(4, qMax(fm.width(headers.at(4) + "   "), fm.width(tr("Downloading  ") + "   ")));
    header->resizeSection(5, qMax(fm.width(headers.at(5) + "   "), fm.width(tr("--:--") + "   ")));

    // Create common actions
    QAction *newJobAction = new QAction(QIcon(":/ui/icons/bottom.png"), tr("Add &new job"), this);
    resuMeJobAction = new QAction(QIcon(":/ui/icons/bottom.png"), tr("&Resume job"), this);
    pauseJobAction = new QAction(QIcon(":/ui/icons/player_pause.png"), tr("&Pause job"), this);
    removeJobAction = new QAction(QIcon(":/ui/icons/player_stop.png"), tr("&Remove job"), this);
    openDirAction = new QAction(QIcon(":/ui/icons/folder.png"), tr("Open file &directory"), this);

    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newJobAction);
    fileMenu->addAction(resuMeJobAction);
    fileMenu->addAction(pauseJobAction);
    fileMenu->addAction(removeJobAction);
    fileMenu->addAction(openDirAction);
    fileMenu->addSeparator();
    fileMenu->addAction(QIcon(":/ui/icons/exit.png"), tr("E&xit"), this, SLOT(close()));

    // Help Menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About"), this, SLOT(about()));

    // Top toolbar
    QToolBar *topBar = new QToolBar(tr("Tools"));
    addToolBar(Qt::TopToolBarArea, topBar);
    topBar->setMovable(false);
    topBar->addAction(newJobAction);
    topBar->addAction(resuMeJobAction);
    topBar->addAction(pauseJobAction);
    topBar->addAction(removeJobAction);
    topBar->addAction(openDirAction);
    pauseJobAction->setEnabled(false);
    removeJobAction->setEnabled(false);
    openDirAction->setEnabled(false);

    // Set up connections
    connect(jobView, SIGNAL(itemSelectionChanged()),
            this, SLOT(setActionsEnabled()));
    connect(newJobAction, SIGNAL(triggered()),
            this, SLOT(addJob()));
    connect(resuMeJobAction, SIGNAL(triggered()),
            this, SLOT(resuMeJob()));
    connect(pauseJobAction, SIGNAL(triggered()),
            this, SLOT(pauseJob()));
    connect(removeJobAction, SIGNAL(triggered()),
            this, SLOT(removeJob()));
    connect(openDirAction, SIGNAL(triggered()),
            this, SLOT(openDir()));

    // Resume form mg!s
    QStringList mgs = db.getAllPaths();
    for (int i = 0; i < mgs.size(); i++)
    {
        resuMeJob(mgs[i]);
    }
}

MainWindow::~MainWindow()
{
    for (int i = 0; i < jobs.size(); i++)
    {
        delete jobs[i].downloader;
    }
}

void MainWindow::setActionsEnabled()
{
    // Find the view item and downloader for the current row, and update
    // the states of the actions.
    QTreeWidgetItem *item = 0;
    if (!jobView->selectedItems().isEmpty())
    {
        item = jobView->selectedItems().first();
    }
    Downloader *downloader = item ? jobs.at(jobView->indexOfTopLevelItem(item)).downloader : 0;
    bool pauseEnabled = downloader && (downloader->getState() == Status::Downloading || downloader->getState() == Status::Paused /* for resume */);
    bool openDirEnabled = downloader && downloader->getState() != Status::Idle;

    openDirAction->setEnabled(item != 0 && openDirEnabled);
    pauseJobAction->setEnabled(item != 0 && pauseEnabled);

    if (downloader && downloader->getState() != Status::Pausing && downloader->getState() != Status::Starting)
    {
        removeJobAction->setEnabled(item != 0);
    }
    else
    {
        removeJobAction->setEnabled(false);
    }

    if (downloader && downloader->getState() == Status::Paused)
    {
        pauseJobAction->setIcon(QIcon(":/ui/icons/player_play.png"));
        pauseJobAction->setText(tr("Resume job"));
    }
    else
    {
        pauseJobAction->setIcon(QIcon(":/ui/icons/player_pause.png"));
        pauseJobAction->setText(tr("Pause job"));
    }
}

QSize MainWindow::sizeHint() const
{
    const QHeaderView *header = jobView->header();

    // Add up the sizes of all header sections. The last section is
    // stretched, so its size is relative to the size of the width;
    // instead of counting it, we count the size of its largest value.
    int width = fontMetrics().width(tr("Remaining time") + "         ");
    for (int i = 0; i < header->count() - 1; ++i)
        width += header->sectionSize(i);

    return QSize(width, QMainWindow::sizeHint().height())
        .expandedTo(QApplication::globalStrut());
}

void MainWindow::set_newJobFileName(QString newFileName)
{
    newJobFileName = newFileName;
}

void MainWindow::set_newJobDownloadedDirectory(QString newDownDir)
{
    newJobDestinationDirectory = newDownDir;
}

bool MainWindow::addJob()
{
    NewTask *newTask = new NewTask(this);

    connect(newTask, SIGNAL(newJob(QString, QString, QString, int)),
            this, SLOT(addJob(QString, QString, QString, int)));
    newTask->show();
    return true;
}

void MainWindow::resuMeJob()
{
    NewTask *newTask = new NewTask(this);

    connect(newTask, SIGNAL(resumeJob(QString)),
            this, SLOT(resuMeJob(QString)));
    newTask->show();
}

void MainWindow::resuMeJob(QString tempFilePath)
{
    QString DownDir = tempFilePath;
    DownDir.chop(tempFilePath.size() - tempFilePath.lastIndexOf(QDir::separator()));

    QString fileName = tempFilePath.right(tempFilePath.size() - tempFilePath.lastIndexOf(QDir::separator()) - 1);
    fileName.chop(strlen(".mg!"));

    // Check if the job is already being downloaded.
    for (int i = 0; i < jobs.size(); i++)
    {
        if (jobs[i].tempFilePath == tempFilePath)
        {
            QMessageBox::warning(this, tr("Already downloading"),
                                 tr("The file %1 is already being downloaded.")
                                     .arg(fileName));
            return;
        }
    }

    qDebug() << DownDir << "ttt" << fileName << endl;

    Downloader *downloader = new Downloader(db, this);

    downloader->setLocalFileName(fileName);
    downloader->setLocalDirectory(DownDir);

    // Setup the downlader connections
    connect(downloader, SIGNAL(stateChanged(QString)),
            this, SLOT(updateState(QString)));
    connect(downloader, SIGNAL(set_GuiProgressBarValue(int)),
            this, SLOT(updateProgress(int)));
    connect(downloader, SIGNAL(set_GuiLabelSpeed(QString)),
            this, SLOT(updateDownloadRate(QString)));
    connect(downloader, SIGNAL(set_GuiLabelDownloaded(QString)),
            this, SLOT(updateDownloaded(QString)));
    connect(downloader, SIGNAL(set_GuiLabelRemainingTime(QString)),
            this, SLOT(updateRemainingTime(QString)));
    connect(downloader, SIGNAL(errorHappened(QString)), this, SLOT(on_error_happens(QString)));

    // Add the downloader to the list of downloading jobs.
    Job job;
    job.downloader = downloader;
    job.fileName = fileName;
    job.destinationDir = DownDir;
    jobs << job;

    // Create and add a row in the job view for this download.
    QTreeWidgetItem *item = new QTreeWidgetItem(jobView);

    QString baseFileName = QFileInfo(fileName).baseName();

    item->setText(0, baseFileName);
    item->setToolTip(0, tr("File: %1<br>Destination: %2")
                            .arg(baseFileName)
                            .arg(DownDir));
    item->setText(1, tr("0/0"));
    item->setText(2, "0");
    item->setText(3, "0.0 KB/s");
    item->setText(4, tr("Idle"));
    item->setText(5, "--:--");
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(1, Qt::AlignHCenter);

    downloader->resumeMyself(tempFilePath);
}

void MainWindow::addJob(QString fileName, QString DownDir, QString URL, int threadNUM)
{
    // Check if the job is already being downloaded.
    for (int i = 0; i < jobs.size(); i++)
    {
        if (jobs[i].fileName == fileName && jobs[i].destinationDir == DownDir)
        {
            QMessageBox::warning(this, tr("Already downloading"),
                                 tr("The file %1 is already being downloaded.")
                                     .arg(fileName));
            return;
        }
    }

    //Create a new downloader
    Downloader *downloader = new Downloader(db, this);
    downloader->setLocalFileName(fileName);
    downloader->setLocalDirectory(DownDir);
    downloader->setThreadNum(threadNUM);

    // Setup the downlader connections
    connect(downloader, SIGNAL(stateChanged(QString)),
            this, SLOT(updateState(QString)));
    connect(downloader, SIGNAL(set_GuiProgressBarValue(int)),
            this, SLOT(updateProgress(int)));
    connect(downloader, SIGNAL(set_GuiLabelSpeed(QString)),
            this, SLOT(updateDownloadRate(QString)));
    connect(downloader, SIGNAL(set_GuiLabelDownloaded(QString)),
            this, SLOT(updateDownloaded(QString)));
    connect(downloader, SIGNAL(set_GuiLabelRemainingTime(QString)),
            this, SLOT(updateRemainingTime(QString)));
    connect(downloader, SIGNAL(errorHappened(QString)), this, SLOT(on_error_happens(QString)));

    // Add the downloader to the list of downloading jobs.
    Job job;
    job.downloader = downloader;
    job.fileName = fileName;
    job.destinationDir = DownDir;
    jobs << job;

    // Create and add a row in the job view for this download.
    QTreeWidgetItem *item = new QTreeWidgetItem(jobView);

    QString baseFileName = QFileInfo(fileName).baseName();

    item->setText(0, baseFileName);
    item->setToolTip(0, tr("File: %1<br>Destination: %2")
                            .arg(baseFileName)
                            .arg(DownDir));
    item->setText(1, tr("0/0"));
    item->setText(2, "0");
    item->setText(3, "0.0 KB/s");
    item->setText(4, tr("Idle"));
    item->setText(5, "--:--");
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setTextAlignment(1, Qt::AlignHCenter);

    downloader->runMyself(URL);
}

void MainWindow::removeJob()
{
    // Find the row of the current item, and find the downloader
    // for that row
    int row = jobView->indexOfTopLevelItem(jobView->currentItem());
    Downloader *dloader = jobs.at(row).downloader;

    // Stop the downloader.
    dloader->stop();
    db.removeTask(QString(dloader->get_localMg().c_str()));

    // Remove the row from the view.
    delete jobView->takeTopLevelItem(row);
    jobs.removeAt(row);
    setActionsEnabled();
}

void MainWindow::pauseJob()
{
    // Pause or unpause the current job.
    int row = jobView->indexOfTopLevelItem(jobView->currentItem());
    Downloader *dloader = jobs.at(row).downloader;
    dloader->setPaused(dloader->getState() != Status::Paused);
    setActionsEnabled();
}

void MainWindow::openDir()
{
    // Open the save directory of the current job.
    int row = jobView->indexOfTopLevelItem(jobView->currentItem());
    QDesktopServices::openUrl(QUrl("file:///" + jobs.at(row).destinationDir));
    setActionsEnabled();
}

void MainWindow::moveJobUp()
{
}

void MainWindow::moveJobDown()
{
}

int MainWindow::rowOfDownloader(Downloader *dloader) const
{
    // Return the row that displays this downloader's status or -1
    // if the downloader is not known.
    int row = 0;
    for(int i = 0; i < jobs.size(); i++)
    {
        if (jobs[i].downloader == dloader)
            return row;
        ++row;
    }
    return -1;
}

void MainWindow::updateState(QString state)
{
    // Update the state string whenever the downloader's state changes
    Downloader *dloader = qobject_cast<Downloader *>(sender());
    int row = rowOfDownloader(dloader);
    QTreeWidgetItem *item = jobView->topLevelItem(row);
    if (item)
    {
        item->setToolTip(0, tr("File: %1<br>Destination: %2<br>State: %3")
                                .arg(jobs.at(row).fileName)
                                .arg(jobs.at(row).destinationDir)
                                .arg(state));

        item->setText(4, state);
    }
    setActionsEnabled();
}

void MainWindow::updateProgress(int percent)
{
    Downloader *dloader = qobject_cast<Downloader *>(sender());
    int row = rowOfDownloader(dloader);

    // Update the progressbar.
    QTreeWidgetItem *item = jobView->topLevelItem(row);
    if (item)
        item->setText(2, QString::number(percent) + QString("%"));
}

void MainWindow::updateDownloadRate(QString speed)
{
    Downloader *dloader = qobject_cast<Downloader *>(sender());
    int row = rowOfDownloader(dloader);

    // Update the download rate.
    QTreeWidgetItem *item = jobView->topLevelItem(row);
    if (item)
        item->setText(3, speed);
}

void MainWindow::updateDownloaded(QString downloaded)
{
    Downloader *dloader = qobject_cast<Downloader *>(sender());
    int row = rowOfDownloader(dloader);

    QString result = downloaded + "/" + dloader->getTotalSize();
    QTreeWidgetItem *item = jobView->topLevelItem(row);
    if (item)
        item->setText(1, result);
}

void MainWindow::updateRemainingTime(QString remainingTime)
{
    Downloader *dloader = qobject_cast<Downloader *>(sender());
    int row = rowOfDownloader(dloader);

    QTreeWidgetItem *item = jobView->topLevelItem(row);
    if (item)
        item->setText(5, remainingTime);
}

const Downloader *MainWindow::downloaderForRow(int row) const
{
    return jobs.at(row).downloader;
}

void MainWindow::on_error_happens(QString errorMsg)
{
    // When the downloading succeeds m_has_error_happend should keep to be false.
    if (!errorMsg.contains(tr("Download successfully in")))
    {
        m_has_error_happend = true;
        m_eMsgBox.DisplayError(errorMsg);
        qCritical() << "Error happened: " << errorMsg << endl;
    }
    else
    {
        m_has_error_happend = false;
    }
}

void MainWindow::closeEvent(QCloseEvent * /*event*/)
{
    qDebug() << "closeEvent is called";
}

void MainWindow::about()
{
    QString about;
    about = tr("mDownloader: A GUI download accelerator.");
    about += QChar::LineSeparator;
    about += tr("Version: 1.1.1Build002.");
    about += QChar::LineSeparator;
    about += tr("Written by Chuan Qin. Email: qc2105@qq.com");
    about += QChar::LineSeparator;
    about += tr("It is based on Qt, and licensed under GPL.");
    msgBox.setWindowTitle(tr("mDownloader"));
    msgBox.setText(about);
    msgBox.exec();
}
