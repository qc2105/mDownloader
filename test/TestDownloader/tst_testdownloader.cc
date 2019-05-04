#include <QString>
#include <QtTest>
#include <QThread>
#include <QSignalSpy>
#include "../../downloader.h"
#include "hashworker.h"


#define TEST_HASH "86d14c0b3d33a8d5a1ae87b55402a6f0"
#define TEST_URL "localhost:8000/test.test"
#define TEMP_FILE "test.tmp"
#define REPEAT_TIMES 1

class TestDownloader : public QObject
{
    Q_OBJECT

public:
    TestDownloader();
    ~TestDownloader();
    QThread hashWorkerThread;

private Q_SLOTS:
    void testDownloader_data();
    void testDownloader();
    void init();
    void cleanup();

signals:
    void operate(QString, QString);

public slots:
    void on_done();
    void setHash(QString);

private:
    QString m_hash;
    Downloader *dloader;
    HashWorker *worker;
};

TestDownloader::TestDownloader()
{
    m_hash = QString("");
}

TestDownloader::~TestDownloader()
{
}

void TestDownloader::on_done()
{
    emit operate(TEMP_FILE, "Md5");
}

void TestDownloader::setHash(QString hashVal)
{
    qDebug() << "setHash called: " << hashVal;
    m_hash = hashVal;
}


void TestDownloader::init()
{
    QFile file(TEMP_FILE);
    QCOMPARE(file.exists(), false);
}

void TestDownloader::cleanup()
{
    QFile file(TEMP_FILE);
    file.remove();
}

void TestDownloader::testDownloader_data()
{
    QTest::addColumn<QString>("URLVal");
    QTest::addColumn<QString>("nThreads");
    QTest::addColumn<QString>("hashVal");

    QString https_qstr = QString("https://") + QString(TEST_URL);
    QString http_qstr = QString("http://") + QString(TEST_URL);
    QString ftp_qstr = QString("ftp://") + QString(TEST_URL);

    for(int i = 0; i < REPEAT_TIMES; i++)
    {
        QString nThreads;
        nThreads.setNum(i);

        QString https_string = QString("https_") + nThreads + QString("_thread");
        QString http_string = QString("http_") + nThreads + QString("_thread");
        QString ftp_string = QString("ftp_") + nThreads + QString("_thread");

//        QTest::newRow(https_string.toStdString().c_str()) << https_qstr    << nThreads  << TEST_HASH;
        QTest::newRow(http_string.toStdString().c_str()) << http_qstr     << nThreads  << TEST_HASH;
//        QTest::newRow(ftp_string.toStdString().c_str()) << ftp_qstr      << nThreads  << TEST_HASH;
    }
}

void TestDownloader::testDownloader()
{
    m_hash = QString("");

    QFETCH(QString, URLVal);
    QFETCH(QString, nThreads);
    QFETCH(QString, hashVal);

    dloader = new Downloader();
    dloader->setLocalDirectory(QDir::currentPath());
    dloader->setLocalFileName(TEMP_FILE);
    dloader->setThreadNum(nThreads.toInt());
    connect(dloader, SIGNAL(done()), this, SLOT(on_done()));

    dloader->runMyself(URLVal);

    worker = new HashWorker;
    worker->moveToThread(&hashWorkerThread);
    connect(&hashWorkerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(this, SIGNAL(operate(QString,QString)), worker, SLOT(doHashWork(QString,QString)));
    connect(worker, SIGNAL(resultReady(QString)), this, SLOT(setHash(QString)), Qt::BlockingQueuedConnection);
    hashWorkerThread.start();

    QSignalSpy spy_on_worker(worker, SIGNAL(resultReady(QString)));
    while (spy_on_worker.count() == 0)
    {
        QTest::qWait(200);
    }

    worker->removeFile();

    hashWorkerThread.terminate();
    delete worker;
    delete dloader;

    QCOMPARE(m_hash , QString(hashVal));
}


QTEST_MAIN(TestDownloader)

#include "tst_testdownloader.moc"
