#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

DbManager::DbManager()
{
    ;
}

DbManager::DbManager(const QString &path)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open())
    {
        qDebug() << "Error: connection with database fail";
    }
    else
    {
        qDebug() << "Database: connection ok";
    }
}

DbManager::~DbManager()
{
    if (m_db.isOpen())
    {
        m_db.close();
    }
}

bool DbManager::isOpen() const
{
    return m_db.isOpen();
}

bool DbManager::createTable()
{
    bool success = false;

    QSqlQuery query;
    query.prepare("CREATE TABLE task(id INTEGER PRIMARY KEY, mg_file_path TEXT);");

    if (!query.exec())
    {
        qDebug() << "Couldn't create the table 'mg_file_path': one might already exist.";
        success = false;
    }

    return success;
}

bool DbManager::addTask(const QString& mg_file_path)
{
    bool success = false;
    if (taskExists(mg_file_path))
    {
        return false;
    }

    if (!mg_file_path.isEmpty())
    {
        QSqlQuery queryAdd;
        queryAdd.prepare("INSERT INTO task (mg_file_path) VALUES (:mg_file_path)");
        queryAdd.bindValue(":mg_file_path", mg_file_path);

        if(queryAdd.exec())
        {
            success = true;
        }
        else
        {
            qDebug() << "add task failed: " << queryAdd.lastError();
        }
    }
    else
    {
        qDebug() << "add task failed: mg_file_path cannot be empty";
    }

    return success;
}

bool DbManager::removeTask(const QString& mg_file_path)
{
    bool success = false;
    qDebug() << "remove: " << mg_file_path;
    if (taskExists(mg_file_path))
    {
        QSqlQuery queryDelete;
        queryDelete.prepare("DELETE FROM task WHERE mg_file_path = (:mg_file_path)");
        queryDelete.bindValue(":mg_file_path", mg_file_path);
        success = queryDelete.exec();

        if(!success)
        {
            qDebug() << "remove task failed: " << queryDelete.lastError();
        }
    }
    else
    {
        qDebug() << "remove task failed: task doesn't exist";
    }

    return success;
}

void DbManager::printAllTasks() const
{
    qDebug() << "Tasks in db:";
    QSqlQuery query("SELECT * FROM task");
    int idName = query.record().indexOf("mg_file_path");
    while (query.next())
    {
        QString name = query.value(idName).toString();
        qDebug() << "===" << name;
    }
}

QStringList DbManager::getAllPaths()
{
    QStringList paths;
    QSqlQuery query("SELECT * FROM task");
    int idName = query.record().indexOf("mg_file_path");
    while (query.next())
    {
        QString name = query.value(idName).toString();
        qDebug() << "===" << name;
        paths.append(name);
    }

    return paths;
}

bool DbManager::taskExists(const QString& mg_file_path) const
{
    bool exists = false;

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT mg_file_path FROM task WHERE mg_file_path = (:mg_file_path)");
    checkQuery.bindValue(":mg_file_path", mg_file_path);

    if (checkQuery.exec())
    {
        if (checkQuery.next())
        {
            exists = true;
        }
    }
    else
    {
        qDebug() << "task exists failed: " << checkQuery.lastError();
    }

    return exists;
}

bool DbManager::removeALLTasks()
{
    bool success = false;

    QSqlQuery removeQuery;
    removeQuery.prepare("DELETE FROM task");

    if (removeQuery.exec())
    {
        success = true;
    }
    else
    {
        qDebug() << "remove all tasks failed: " << removeQuery.lastError();
    }

    return success;
}
