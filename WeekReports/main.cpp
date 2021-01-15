#include "widget.h"
#include "reportform.h"
#include "miningareamap.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ReportForm w;
    w.showMaximized();
   // MiningAreaMap::getMiningAreaMapConfigure();
    return a.exec();
}
