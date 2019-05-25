#include <iostream>
#include <gtest/gtest.h>
#include "../src/dbmanager.h"

using namespace std;
using ::testing::InitGoogleTest;
using ::testing::Test;

class dbTest : public testing::Test
{
protected:
    void SetUp() override
    {
        db.createTable();
    }
    void TearDown() override
    {
        db.removeALLTasks();
    }
    DbManager db = DbManager("mDownloader.db");
    QString path1 = "/home/qc/test1.mg!";
    QString path2 = "/home/qc/test2.mg!";
    QString path3 = "/home/qc/test3.mg!";
};

TEST_F(dbTest, test_constructor)
{
    ASSERT_TRUE(db.isOpen());
}

TEST_F(dbTest, test_addTask)
{
    db.addTask(path1);
    ASSERT_TRUE(db.taskExists(path1));
    bool ret = db.addTask(path1);
    ASSERT_FALSE(ret);
}

TEST_F(dbTest, test_removeTask)
{
    db.removeTask(path1);
    ASSERT_FALSE(db.taskExists(path1));
}

TEST_F(dbTest, test_removeAllTasks)
{
    db.addTask(path1);
    db.addTask(path2);
    db.removeALLTasks();
    ASSERT_FALSE(db.taskExists(path1));
    ASSERT_FALSE(db.taskExists(path2));
}

TEST_F(dbTest, test_getAllPaths)
{
    db.addTask(path1);
    db.addTask(path2);
    db.addTask(path3);
    QStringList paths = db.getAllPaths();
    ASSERT_STREQ(paths[0].toStdString().c_str(), path1.toStdString().c_str());
    ASSERT_STREQ(paths[1].toStdString().c_str(), path2.toStdString().c_str());
    ASSERT_STREQ(paths[2].toStdString().c_str(), path3.toStdString().c_str());
}

int main()
{
    InitGoogleTest();

    return RUN_ALL_TESTS();
}
