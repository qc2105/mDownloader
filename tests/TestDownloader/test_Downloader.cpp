#include <QString>
#include <QDir>
#include <QApplication>
#include <QCryptographicHash>
#include <QFile>


#include <iostream>
#include <gtest/gtest.h>

#include "downloader.h"
#include "dbmanager.h"

using namespace std;
using ::testing::InitGoogleTest;
using ::testing::Test;

#define TEST_HASH "58613b2e077e1fff7e134f1a1c42bf0c"
#define TEST_URL "http://localhost:8000/test.test"
#define TEMP_FILE "test.tmp"
#define REPEAT_TIMES 1

class downloaderTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_hash = QString("");
        QFile file(TEMP_FILE);
        ASSERT_FALSE(file.exists());
        m_db.createTable();
    }
    void TearDown() override
    {
        m_db.removeALLTasks();
        QFile file(TEMP_FILE);
        file.remove();
    }
    DbManager m_db = DbManager("testDownloader.db");
    QString m_hash;
    Downloader *dloader;
    QFile *m_file;

public :
    void setHash(QString);
    QString hash(QString fileName, QString hashAlgorithm);
};

QString downloaderTest::hash(QString FileName, QString HashAlgorithm)
{
    QByteArray resultByteArray;
    QByteArray hashHexByteArray;
    QString resultQString = "";
    QCryptographicHash *hash = NULL;

    m_file = new QFile(FileName);

    if (!m_file->exists() || m_file->size() == 0)
    {
        return QString("no such file");
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
        while (!m_file->atEnd()) {
            hash->addData(m_file->read(readingBlockSize));
        }
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

    return resultQString;
}

void downloaderTest::setHash(QString hashVal)
{
    qDebug() << "setHash called: " << hashVal;
    m_hash = hashVal;
}

TEST_F(downloaderTest, testIntegrity)
{
    m_hash = QString("");

    dloader = new Downloader(m_db);
    dloader->setLocalDirectory(QDir::currentPath());
    dloader->setLocalFileName(TEMP_FILE);
    dloader->setThreadNum(1);

    dloader->runMyself(TEST_URL);
   
    int timeout = 10;
    while (dloader->getState() != Status::DownloadStatus::Finished && timeout <= 10)
    {
        sleep(1);
        timeout++;
    }

    m_hash = hash(QString(TEMP_FILE), "Md5");
    string result = m_hash.toStdString();
    string expected = QString(TEST_HASH).toStdString();

    ASSERT_STREQ(result.c_str(), expected.c_str());
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    InitGoogleTest();

    return RUN_ALL_TESTS();
}

