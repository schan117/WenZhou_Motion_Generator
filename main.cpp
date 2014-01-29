#include "mainwindow.h"
//#include <QApplication>
#include <QtSingleApplication>

int main(int argc, char *argv[])
{

    QtSingleApplication app(__argc, __argv);

    QStringList args = app.arguments();

    QString message;

    if (args.size() == 1)
    {
        message = "raise";
    }
    else
    {
        message = args[1];
    }

    if (app.isRunning())
    {
        if (app.sendMessage(message) || app.isRunning())
        {
            return 0;
        }
    }

    app.setFont(QFont("Calibri", 14));

    QTranslator ts;
    QTranslator ts1;

    QString locale = QLocale::system().name();
    qDebug() << "Locale: " << locale;

    //if (locale.startsWith("zh"))
    //{
   ts.load("Metron_Motion_Generator_zh_CN.qm");
   ts1.load("qt_zh_CN.qm");
    app.installTranslator(&ts1);
    app.installTranslator(&ts);


//    QApplication app(argc, argv);

    MainWindow w;

    return app.exec();
}
