#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QStringList>

/**
 * \class DbManager
 *
 * \brief SQL Database Manager class
 *
 * DbManager sets up the connection with SQL database
 * and performs some basics queries.
 */
class DbManager
{
public:
    DbManager();
    /**
     * @brief Constructor
     *
     * Constructor sets up connection with db and opens it
     * @param path - absolute path to db file
     */
    DbManager(const QString &path);

    /**
     * @brief Destructor
     *
     * Close the db connection
     */
    ~DbManager();

    bool isOpen() const;

    bool createTable();

    bool addTask(const QString &mg_file_path);

    bool removeTask(const QString &mg_file_path);

    bool taskExists(const QString &mg_file_path) const;

    void printAllTasks() const;

    QStringList getAllPaths();

    bool removeALLTasks();

private:
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
