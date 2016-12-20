#include "printerdialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PrinterDialog w;
    w.show();

    return a.exec();
}
