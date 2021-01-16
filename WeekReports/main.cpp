#include "widget.h"
#include "reportform.h"
#include "miningarealocation.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ReportForm w;
    w.showMaximized();
    return a.exec();
}
