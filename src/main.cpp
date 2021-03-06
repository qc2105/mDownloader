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

#include "mainwindow.h"
#include "newtask.h"
#include "myget.h"

#include <QObject>
#include <QApplication>
#include <QDesktopWidget>
#include <QTranslator>
#include <QLibraryInfo>


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFile debug_log("debug.log");
    QFile error_log("error.log");

    if (!debug_log.open(QIODevice::Append | QIODevice::Text))
          return;
    if (!error_log.open(QIODevice::Append | QIODevice::Text))
          return;

    QTextStream debug_out(&debug_log);
    QTextStream error_out(&error_log);

    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtInfoMsg:
        debug_out << "[Info:] " << localMsg.constData() << "(" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        break;
    case QtDebugMsg:
        debug_out << "[Debug:] " << localMsg.constData() << "(" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        break;
    case QtWarningMsg:
        debug_out << "[Warning:] " << localMsg.constData() << "(" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        break;
    case QtCriticalMsg:
        error_out << "[Critical:] " << localMsg.constData() << "(" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        break;
    case QtFatalMsg:
        error_out << "[Fatal:] " << localMsg.constData() << "(" << context.file << ":" << context.line << ", " << context.function << ")" << endl;
        abort();
    }
}

#ifdef WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{   
    if (!QLibraryInfo::isDebugBuild())
    {
        qInstallMessageHandler(myMessageOutput);
    }

    QCoreApplication::setOrganizationName("QC2105");
    QCoreApplication::setApplicationName("mDownloader");
	
#ifdef WIN32
	int argc = 0;
	QApplication a(argc, NULL);
#else
    QApplication a(argc, argv);
#endif
    QTranslator qtTranslator;
       qtTranslator.load("qt_" + QLocale::system().name(),
               QLibraryInfo::location(QLibraryInfo::TranslationsPath));
       a.installTranslator(&qtTranslator);

       QTranslator myappTranslator;
       myappTranslator.load("mDownloader_" + QLocale::system().name());
       a.installTranslator(&myappTranslator);

    a.setQuitOnLastWindowClosed(true);

    QDesktopWidget qd;

    MainWindow w;

    w.move((qd.availableGeometry(-1).width()-w.width())/2, (qd.availableGeometry(-1).height()-w.height())/2);
    w.show();


    return a.exec();
}

