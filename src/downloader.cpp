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

#include <sys/stat.h>
#include <errno.h>

#include "downloader.h"
#include "macro.h"
#include "utils.h"
#include "ftpplugin.h"
#include "httpplugin.h"
#include "progressbar.h"
#include "debug.h"
#include "header.h"

#include <QDir>
#include <QByteArray>

#if WIN32
#define snprintf sprintf_s
#endif

typedef void *(*threadFunction)(void *);

Downloader::Downloader(DbManager &_db, QWidget *parent) : QThread(parent),
                                                          db(_db)
{
    m_status = new Status(this);
    setState(Status::Idle);
    stateString = QT_TRANSLATE_NOOP(Downloader, "Idle");
    plugin = NULL;
    blocks = NULL;
    toTalSize = QString("-1");
    localPath = NULL;
    localMg = NULL;
    pb = new ProgressBar;
    is_dirSetted = false;
    emit set_GuiProgressBarMinimum(0);
}

Downloader::~Downloader(void)
{
    delete[] blocks;
    delete[] localPath;
    delete[] localMg;
    delete pb;
    delete plugin;
}

int Downloader::init_plugin(void)
{
    if (task.get_proxy_type() == HTTP_PROXY)
    {
        delete plugin;
        plugin = new HttpPlugin;
    }
    else
    {
        switch (task.get_protocol())
        {
        case HTTP:
        case HTTPS:
            delete plugin;
            plugin = new HttpPlugin;
            break;
        case FTP:
            delete plugin;
            plugin = new FtpPlugin;
            break;
        default:
            return -1;
        }
    }

    return 0;
};

/* init_plugin, plugin->get_info */
int Downloader::init_task(void)
{
    int i;
    int ret;

_reinit_plugin:
    if (init_plugin() < 0)
    {
        qCritical() << "Unknown protocol" << endl;
        return -1;
    }

    for (i = 0; task.get_tryCount() <= 0 || i < task.get_tryCount(); i++)
    {
        ret = plugin->get_info(&task);
        if (ret == -1)
        {
            qCritical() << "Plugin->get_info return -1" << endl;
            return -1;
        }
        else if (ret == S_REDIRECT)
        {
            qDebug() << "Redirect to: " << task.get_url() << endl;
            goto _reinit_plugin;
        }
        else if (ret == 0)
        {
            return 0;
        }
        else
        {
            continue;
        }
    }
    qDebug() << "return e_max_count" << endl;
    qDebug() << "ret = " << ret << endl;
    return E_MAX_COUNT;
}

int Downloader::init_local_file_name(void)
{
    size_t length;
	std::string tempString;
    
	length = task.get_local_dir() == std::string("") ? 0 : task.get_local_dir().length();
   	length += task.get_local_file() == std::string("") ? strlen(task.get_file()) : task.get_local_file().length();
    length += 6;
	 
	tempString = task.get_local_dir() == std::string("") ? "" : task.get_local_dir() + QString(QDir::separator()).toStdString();
	tempString += task.get_local_file() == std::string("") ? task.get_file() : task.get_local_file();
	tempString += ".mg!";
	
	if (!localPath)
	{
		delete[] localPath;
	}
	if (!localMg)
	{
		delete[] localMg;
	}
    tempString[length - 5] = '\0';
    localPath = StrDup(tempString.c_str());
    tempString[length - 5] = '.';
   	localMg = StrDup(tempString.c_str());

    return 0;
}

// please take a look at save_temp_file_exit() to get more details
int Downloader::init_threads_from_mg(void)
{
    QFile *tempFile = NULL;
    QByteArray readData;
    int i;
    QString error;

    m_status->setDownloadMode(Status::ResumeDownload);

    tempFile = new QFile(localMg);

    if (!tempFile->open(QIODevice::ReadOnly))
    {
        qCritical() << "Can not access the temp file: " << localMg << endl;
        error = QString("Can not access the temp file: ");
        error += QString(localMg);
        emit errorHappened(error);
        delete tempFile;
        return -1;
    }

    qDebug() << "File size immediately after open: " << tempFile->size() << endl;

    // get the length of data in mg! file.
    readData.clear();
    qDebug() << QString("\n").toUtf8().size() + blockSize << endl;
    qDebug() << "teampFile->size(): " << tempFile->size() << endl;
    tempFile->seek(tempFile->size() - QString("\n").toUtf8().size() - blockSize);
    readData.clear();
    readData = tempFile->read(blockSize);
    qint64 fileDataSize = readData.toLongLong();
    qDebug() << "fileDataSize: " << fileDataSize << endl;
    qint64 nextReadPos = fileDataSize;

    // get the url
    tempFile->seek(nextReadPos);
    readData.clear();
    readData = tempFile->read(max_url_length);
    QString readURL = QString::fromUtf8(readData);
    qDebug() << "Read additional size: " << readURL.size() << endl;
    qDebug() << "Read url: " << readURL << endl;
    task.set_url(readURL.toStdString().c_str());
    readData.clear();

    nextReadPos += max_url_length;
    tempFile->seek(nextReadPos);

    // get the number of threads
    readData = tempFile->read(blockSize);
    threadNum = readData.toInt();
    readData.clear();

    qDebug() << "File size: " << tempFile->size() << endl;
    qDebug() << "threadNum: " << threadNum << endl;

    nextReadPos += blockSize;

    // TODO: We'd better use a hash function to check.
    if (tempFile->size() < task.get_file_size() + 3 * blockSize * threadNum)
    {
        qCritical() << "the temp file: \"" << localMg << "\" is not correct, type I." << endl;
        error = QString("The temp file: ");
        error += QString(localMg);
        error += QString(" is not correct, type I.");
        tempFile->close();
        delete tempFile;
        emit errorHappened(error);
        return -1;
    }

    // get the data
    delete[] blocks;
    blocks = new Block[threadNum];
    for (i = 0; i < threadNum; i++)
    {
        readData = tempFile->read(blockSize);
        blocks[i].startPoint = readData.toLong();
        readData.clear();
        qDebug() << "startPoint for thread " << i << ": " << blocks[i].startPoint << endl;
        nextReadPos += blockSize;
        tempFile->seek(nextReadPos);
        readData = tempFile->read(blockSize);
        blocks[i].downloaded = readData.toLong();
        readData.clear();
        qDebug() << "downloaded for thread " << i << ": " << blocks[i].downloaded << endl;
        nextReadPos += blockSize;
        tempFile->seek(nextReadPos);
        readData = tempFile->read(blockSize);
        blocks[i].size = readData.toLong();
        readData.clear();
        nextReadPos += blockSize;
        tempFile->seek(nextReadPos);
        qDebug() << "size for thread " << i << ": " << blocks[i].size << endl;
        if (!(blocks[i].bufferFile.open(localMg)))
        {
            tempFile->close();
            delete tempFile;
            perror("Can not open the temp file to write");
            emit errorHappened(QString("Can not open the temp file to write"));
            return -1;
        }
    }

    tempFile->close();
    delete tempFile; // don't panic, we only delete the QFile object here not the tempFile itself.
    return 0;
}

int Downloader::init_threads_from_info(void)
{
    qint64 block_size;
    int i;

    m_status->setDownloadMode(Status::NewDownload);

    threadNum = task.get_threadNum() > 0 ? task.get_threadNum() : 1;
    block_size = task.get_file_size() / threadNum;
    if (block_size <= 20480)
    { // too small file
        threadNum = 1;
        block_size = task.get_file_size();
    }

    delete[] blocks;
    blocks = new Block[threadNum];
    for (i = 0; i < threadNum; i++)
    {
        blocks[i].startPoint = i * block_size;
        blocks[i].size = block_size;
        if (!(blocks[i].bufferFile.open(localMg)))
        {
            perror("Can not open the temp file to write");
            emit errorHappened(QString("Can not open the temp file to write"));
            return -1;
        }
    }

    blocks[threadNum - 1].size = task.get_file_size() - block_size * (threadNum - 1);

    return 0;
}

int Downloader::thread_create(void)
{
    int i;

    QThread *workerThread = new QThread;
    DownloadWorker *worker = new DownloadWorker;
    worker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    workerThread->start();
    for (i = 0; i < threadNum; i++)
    {
        if (blocks[i].ptr_thread == 0)
        { // found an empty slot
            blocks[i].ptr_thread = workerThread;
            break;
        }
    }

    if (i == threadNum)
        return -1;

    connect(this, SIGNAL(begin(Downloader *, QThread *)), worker, SLOT(doWork(Downloader *, QThread *)));
    emit begin(this, workerThread);
    disconnect(this, SIGNAL(begin(Downloader *, QThread *)), 0, 0);

    return 0;
}

int Downloader::self(QThread *ptr_thread)
{
    QThread *self;
    self = ptr_thread;
    int i;
    while (1)
    {
        for (i = 0; i < threadNum; i++)
        {
            if (blocks[i].ptr_thread == self)
                return i;
        }
        // the parent thread maybe slower than me
    }
}

void Downloader::setState(const Status::DownloadStatus state)
{
    m_status->setDownloadStatus(state);
    switch (state)
    {
    case Status::Idle:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Idle");
        break;
    case Status::Starting:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Starting");
        break;
    case Status::Downloading:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Downloading");
        break;
    case Status::Finished:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Finished");
        break;
    case Status::Failed:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Failed");
        break;
    case Status::Paused:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Paused");
        break;
    case Status::Pausing:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Pausing");
        break;
    case Status::Stopping:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Stopping");
        break;
    default:
        stateString = QT_TRANSLATE_NOOP(Downloader, "Unkown status");
        break;
    }
    emit stateChanged(stateString);
}

Status::DownloadStatus
Downloader::getState() const
{
    return m_status->downloadStatus();
}

QString
Downloader::getStateString() const
{
    return stateString;
}

int Downloader::download_thread(Downloader *downloader, QThread *ptr_thread)
{
    int self, ret;
    self = downloader->self(ptr_thread);
    qDebug() << endl
             << "download_thread start in thread: " << self;

    while (1)
    {
        ret = downloader->plugin->download(downloader->task, downloader->blocks + self);
        if (ret == E_SYS)
        {
            qCritical() << endl
                        << "thread " << self << " plugin download returen E_SYS" << endl;
        }
        else
        {
            qDebug() << endl
                     << "thread " << self << " plugin download returned: " << ret << endl;
        }
        if (ret == E_SYS)
        { // system error
            downloader->blocks[self].state = EXIT;
            return -1;
        }
        else if (ret == 0)
        {
            downloader->blocks[self].state = EXIT;
            return 0;
        }
        else
        {
            continue;
        }
    }

    downloader->blocks[self].state = EXIT;
    return E_MAX_COUNT;
}

int Downloader::schedule(void)
{
    int i, j;
    int joined;

    joined = 0;
    for (i = 0; i < threadNum; i++)
    {
        if (blocks[i].state == WAIT)
        {
            for (j = i + 1; j < threadNum; j++)
            {
                if (blocks[i].startPoint + blocks[i].size == blocks[j].startPoint)
                {
                    break;
                }
            }
            if (j < threadNum && blocks[j].downloaded == 0)
            {
                if (blocks[j].state == STOP || blocks[j].state == EXIT)
                {
                    blocks[j].ptr_thread->quit();
                    blocks[j].state = JOINED;
                    blocks[j].bufferFile.close();
                }
                else if (blocks[j].state != JOINED)
                {
                    continue;
                }
                blocks[i].size += blocks[j].size;
                blocks[i].state = WAKEUP;
                blocks[j].startPoint = -1;
                blocks[j].downloaded = 0;
                blocks[j].size = 0;
                i = j;
                joined += j - i;
                qint64 *data = new qint64[threadNum];
                for (j = 0; j < threadNum; j++)
                {
                    data[j] = blocks[j].startPoint;
                }
                pb->set_start_point(data);

                delete[] data;
            }
        }
        else if (blocks[i].state == EXIT)
        {
            blocks[i].ptr_thread->quit();
            blocks[i].state = JOINED;
            blocks[i].bufferFile.close();
            joined++;
        }
        else if (blocks[i].state == JOINED)
        {
            joined++;
        }
        else
        {
            continue;
        }
    }

    return threadNum - joined;
}

int Downloader::remove_temp_file_exit(void)
{
    int i;
    QFile *tempFile;

    for (i = 0; i < threadNum; i++)
    {
        if (blocks[i].state != JOINED)
        {
            blocks[i].ptr_thread->quit();
            blocks[i].ptr_thread->terminate();
            blocks[i].ptr_thread->wait();
            blocks[i].bufferFile.close();
            blocks[i].state = STOP;
        }
    };

    tempFile = new QFile(localMg);

    tempFile->remove();
    delete tempFile;
    db.removeTask(localMg);

    return 0;
}

int Downloader::save_temp_file_exit(void)
{
    int i;
    QFile *tempFile;
    QByteArray writeData;
    QString error;

    for (i = 0; i < threadNum; i++)
    {
        if (blocks[i].state != JOINED)
        {
            blocks[i].ptr_thread->quit();
            blocks[i].ptr_thread->terminate();
            blocks[i].ptr_thread->wait();
            blocks[i].bufferFile.close();
            blocks[i].state = STOP;
        }
    };

    if (task.get_file_size() < 0)
    {
        qCritical() << "!!!You can not continue in further" << endl;
        setState(Status::Failed);
        exit(-1); // Is this right?
    }


    tempFile = new QFile(localMg);
    if (!tempFile->open(QIODevice::ReadWrite))
    {
        qCritical() << "Can not create the temp file: " << localMg << endl;
        error = QString("Can not create the temp file: ");
        error += QString(localMg);
        emit errorHappened(error);
        delete tempFile;
        return -1;
    }

    qint64 nextWritePos = task.get_file_size();
    qDebug() << "nextWritePos: " << nextWritePos << endl;

    tempFile->seek(nextWritePos);

    // save url
    writeData.clear();
    writeData = QString(task.get_url()).toUtf8();
    int write_size = tempFile->write(writeData);
    qDebug() << "write additional size: " << write_size << endl;
    qDebug() << "Write url: " << QString::fromUtf8(writeData) << endl;
    writeData.clear();

    nextWritePos += max_url_length;

    tempFile->seek(nextWritePos);

    // save number of threads
    writeData.clear();
    writeData.setNum(threadNum);
    tempFile->write(writeData);
    qDebug() << "Write threadNum: " << writeData.toInt() << endl;
    writeData.clear();

    nextWritePos += blockSize;
    tempFile->seek(nextWritePos);

    // save data
    for (i = 0; i < threadNum; i++)
    {
        writeData.setNum(blocks[i].startPoint);
        tempFile->write(writeData);
        writeData.clear();
        qDebug() << "Write startPoint for thread " << i << ": " << blocks[i].startPoint << endl;
        nextWritePos += blockSize;
        tempFile->seek(nextWritePos);
        writeData.setNum(blocks[i].downloaded);
        tempFile->write(writeData);
        writeData.clear();
        qDebug() << "Write downloaded for thread " << i << ": " << blocks[i].downloaded << endl;
        nextWritePos += blockSize;
        tempFile->seek(nextWritePos);
        writeData.setNum(blocks[i].size);
        tempFile->write(writeData);
        writeData.clear();
        qDebug() << "Write size for thread " << i << ": " << blocks[i].size << endl;
        nextWritePos += blockSize;
        tempFile->seek(nextWritePos);
    }

    // save size of data in file
    writeData.setNum(task.get_file_size());
    tempFile->write(writeData);
    qDebug() << "Write fileDataSize: " << writeData.toLongLong() << endl;
    writeData.clear();
    nextWritePos += blockSize;
    tempFile->seek(nextWritePos);

    /* save the "marker of the end of a file chunk"
     * we do this because this can make sure we save the fileDataSize above in a fixed length block(its size is blockSize).
     * the marker's size is also fixed, it is QString("\n").toUtf8().size().
     * all these will make the work in init_threads_from_mg() a little easier
     */
    writeData = QString("\n").toUtf8();
    write_size = tempFile->write(writeData);
    qDebug() << "write \n size: " << write_size << endl;
    writeData.clear();

    tempFile->waitForBytesWritten(-1);

    tempFile->close();
    delete tempFile;

    db.addTask(localMg);

    setState(Status::Paused);
    
    return 0;
}

int Downloader::try_resume_from_paused(void)
{
    int ret;
    if (!task.get_resumeSupported() || task.get_file_size() < 0)
    {
        threadNum = 1;
        delete[] blocks;
        blocks = new Block[1];
        blocks[0].size = task.get_file_size();
        blocks[0].bufferFile.open(localMg);
        ret = 0;
    }
    else if (file_exist(localMg))
    {
        ret = init_threads_from_mg();
    }
    else
    {
        ret = init_threads_from_info();
    }
    return ret;
}

int Downloader::create_zero_file(void)
{
    QFile file(localPath);
    if (!file.open(QIODevice::ReadWrite))
    {
        qCritical() << "error when creat file: " << localPath << endl;
        emit errorHappened(QString("error when creat file"));
        return -1;
    }
    else
    {
        file.close();
        qDebug() << "Created data file: " << localPath << endl;
        return 0;
    }
}

int Downloader::is_already_existed(void)
{
    QString errorMsg;
    if (file_exist(localPath))
    {
        qCritical() << "File already exist: " << localPath << endl;
        errorMsg = QString(tr("The file already exists: "));
        errorMsg += QDir::toNativeSeparators(localPath);
        emit errorHappened(errorMsg);
        return 1;
    }
    else
        return 0;
}

int Downloader::create_downloading_threads(void)
{
    for (int i = 0; i < threadNum; i++)
    {
        if (thread_create() < 0)
{
            perror("Create thread failed");
            emit errorHappened("Create thread failed");
            return -1;
        }
    }

    return 0;
}

int Downloader::pre_download_process(double start_time)
{
    int ret = 0;

    init_local_file_name();

    // Already done...
    if (is_already_existed())
        return 3;
    if (task.get_file_size() == 0)
    {
        if ((ret = create_zero_file()) == 0)
            report_done(start_time);
        return ret;
    }

    char buf[20];
    convert_size(buf, task.get_file_size());
    qDebug() << "Filesize: " << buf << endl;
    emit set_GuiLabelTotal(QString(buf));
    toTalSize = QString(buf);

    if ((ret = try_resume_from_paused()) < 0)
    {
        qCritical() << "try_resume_from_paused failed" << endl;
        return ret;
    }

    if (create_downloading_threads() != 0)
    {
        return -1;
    }

    return 0;
}

int Downloader::file_download(void)
{
    double time = get_current_time();
    int pre_return = -100; // -100, a value that's impossible.

    if ((pre_return = pre_download_process(time)) != 0)
    {
        if (pre_return == 3) // file already exists.
        {
            setState(Status::Finished);
            return -3;
        }
        else
        {
            qCritical() << "pre_download_processing failed" << endl;
            return -1;
        }
    }

    // update loop
    prepare_progress_bar();
    setState(Status::Downloading);
    while (1)
    {
        if (getState() == Status::Pausing)
        {
            delete[] pb->data;
            save_temp_file_exit();
            return 0;
        }
        else if (getState() == Status::Stopping)
        {
            delete[] pb->data;
            remove_temp_file_exit();
            return 0;
        }

        for (int i = 0; i < threadNum; i++)
        {
            pb->data[i] = blocks[i].downloaded;
        }
        update_progress_bar();

        if (schedule() == 0)
        {
            qDebug() << "all the thread exited!" << endl;
            break; // all the thread exited
        }
        usleep(250000);
    }
    delete[] pb->data;

    if (post_download_process(time) != 0)
    {
        qCritical() << "post_download_processing failed" << endl;
        return -1;
    }

    return 0;
}

void Downloader::prepare_progress_bar(void)
{
    pb->data = new qint64[threadNum];
    for (int i = 0; i < threadNum; i++)
    {
        pb->data[i] = blocks[i].startPoint;
    }

    pb->init();
    pb->set_total_size(task.get_file_size());
    qDebug() << "file_download: task.fileSize" << task.get_file_size();
    emit set_GuiProgressBarMaximum(100);
    pb->set_block_num(threadNum);
    pb->set_start_point(pb->data);
}

void Downloader::update_progress_bar(void)
{
    pb->update(pb->data);
    float downloadedRatio = (float)(pb->get_curr_downloaded()) / (float)(task.get_file_size()) * 100;
    qDebug() << "downloadedRatio: " << downloadedRatio;
    emit set_GuiProgressBarValue(downloadedRatio);
    emit set_GuiLabelDownloaded(QString(pb->get_downloaded()));
    emit set_GuiLabelSpeed(QString(pb->get_downloadRate()) + QString("/S"));
    emit set_GuiLabelRemainingTime(QString(pb->get_eta()));
}

int Downloader::post_download_process(double download_start_time)
{
    if (check_downloaded_file() != 0)
    {
        return -1;
    }

    if (rename_temp_file() != 0)
    {
        return -1;
    }

    resize_downloaded_file(); // Do we really care about the size?

    report_done(download_start_time);

    db.removeTask(localMg);

    return 0;
}

void Downloader::report_done(double start_time)
{
    char buf[20];
    double time;
    QString errorMsg;

    setState(Status::Finished);
    time = get_current_time() - start_time;
    convert_time(buf, time);
    emit set_GuiProgressBarValue(100);

    qDebug() << endl
             << "Download successfully in " << buf << endl;
    errorMsg = QString(tr("Download successfully in "));
    errorMsg += QString(buf);
    emit errorHappened(errorMsg);
    emit done();
}

int Downloader::resize_downloaded_file(void)
{
    QFile downloadedFile(localPath);

    if (!downloadedFile.resize(task.get_file_size())) // Do we really need !sigint_received?
    {
        qCritical() << "Resize the downloaed file failed" << endl;
        emit errorHappened(QString(tr("Resize the downloaed file failed")));
        return -1;
    }
    return 0;
}

int Downloader::rename_temp_file(void)
{
    QFile file(localMg);

    if (!file.rename(localPath))
    {
        qCritical() << "Rename failed" << endl;
        emit errorHappened(QString(tr("Rename failed")));
        return -1;
    }
    else
    {
        qDebug() << "Rename succeed." << endl;
        return 0;
    }
}

int Downloader::check_downloaded_file(void)
{
    // recheck the size of the file if possible
    if (task.get_file_size() >= 0)
    {
        qint64 downloaded;
        downloaded = 0;
        for (int i = 0; i < threadNum; i++)
        {
            downloaded += blocks[i].downloaded;
        }
        // the downloaded maybe bigger than the filesize
        // because the overlay of the data
        if (downloaded < task.get_file_size())
        {
            qCritical() << "!!!Some error happend when downloaded" << endl;
            qCritical() << "!!!Redownloading is recommended" << endl;
            emit errorHappened("!!!Some error happend when downloaded. !!!Redownloading is recommended");
            save_temp_file_exit();
            return (-1);
        }
    }

    return 0;
}

void Downloader::run(void)
{
    int ret;

    ret = init_task(); // Maybe we could put this init_task() function into the class Task?
    if (ret < 0)
    {
        qCritical() << "Can not get the info of the file " << endl;
        emit errorHappened(QString(tr("Can not get the info of the file ")));
        setState(Status::Failed);

        return;
    }

    if (task.get_isDirectory())
    {
        qDebug() << "This is a directory: " << task.get_url() << endl;
        return;
    }

    file_download();

    return;
}

void Downloader::runMyself(QString QUrl)
{
    if (!QUrl.isEmpty())
{
        task.set_url(QUrl.toUtf8().constData());
    }
    else
    {
        qCritical() << "runMyself: QUrl is empty!" << endl;
    }
    setState(Status::Starting);
    start();
}

void Downloader::resumeMyself(QString tempFilePath)
{
    init_local_file_name();
    if (!tempFilePath.isEmpty())
    {
        init_threads_from_mg();
    }
    else
    {
        qCritical() << "runMyself: tempFilePath is empty!" << endl;
    }
    runMyself(task.get_url());
}

void Downloader::resumeTask(void)
{
    setState(Status::Starting);
    start();
}

void Downloader::setLocalDirectory(QString dir)
{
   	task.set_local_dir(dir.toStdString());
    is_dirSetted = true;
}

void Downloader::setLocalFileName(QString QFileName)
{
    task.set_local_file(QFileName.toUtf8().constData());
}

void Downloader::pause(void)
{
    setState(Status::Pausing);
}

void Downloader::stop(void)
{
    setState(Status::Stopping);
}

void Downloader::setThreadNum(int num)
{
    task.set_threadNum(num);
}

void Downloader::setPaused(bool paused)
{
    if (paused)
    {
        pause();
    }
    else
    {
        resumeTask();
    }
}
