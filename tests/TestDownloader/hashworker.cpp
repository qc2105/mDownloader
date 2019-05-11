#include "hashworker.h"
#include "../../macro.h"
//#include <QDateTime>
#include <QDebug>

HashWorker::HashWorker(QObject *parent) :
    QObject(parent),
    m_currentProgress(0)
{
}

void HashWorker::removeFile()
{
    m_mutex.lock();
    m_file->remove();
    m_mutex.unlock();
}

void HashWorker::doHashWork(QString FileName, QString HashAlgorithm)
{
    m_mutex.lock();
    QByteArray resultByteArray;
    QByteArray hashHexByteArray;
    QString resultQString = "";
    QCryptographicHash *hash = NULL;

    m_file = new QFile(FileName, this);

    if (!m_file->exists() || m_file->size() == 0)
    {
        emit resultReady("File doesn't exist or is empty");
        return;
    }
    else
    {
        qDebug() << m_file->size();
    }

    if(m_file->open(QIODevice::ReadOnly)) {   //ToDo: what if the file is a directory?
        if (HashAlgorithm == "Md5")
        {
            hash = new QCryptographicHash(QCryptographicHash::Md5);
        }
        else if (HashAlgorithm == "Sha1")
        {
            hash = new QCryptographicHash(QCryptographicHash::Sha1);
        }
#ifdef QT5_ABOVE
        else if (HashAlgorithm == "Sha256")
        {
            hash = new QCryptographicHash(QCryptographicHash::Sha256);
        }
#endif
        int steps = 0;
        while (!m_file->atEnd()) {
            hash->addData(m_file->read(readingBlockSize));
            m_currentProgress += readingBlockSize;
            emit currentProgress(++steps);
        }
        emit currentProgress(m_file->size()/readingBlockSize);
        resultByteArray = hash->result();
        hashHexByteArray = resultByteArray.toHex();

        resultQString += hashHexByteArray.data();

    }else {
        resultQString += QString("file open error");
    }

    delete hash;
    if (resultQString.isEmpty()) {
        resultQString += QString("Error, empty resultQString");
    }
    emit resultReady(resultQString);
    m_mutex.unlock();
}
