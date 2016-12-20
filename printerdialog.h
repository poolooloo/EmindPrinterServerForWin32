#ifndef PRINTERDIALOG_H
#define PRINTERDIALOG_H

#include "printerserver.h"
#include <QDialog>
//#include <QPrinter>


namespace Ui {
class PrinterDialog;
}

class PrinterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrinterDialog(QWidget *parent = 0);
    ~PrinterDialog();

private:
    Ui::PrinterDialog *ui;
	PrinterServer printerServer;
	qint16 tcpPort;

private slots:
    void getIP();
    void on_flushButton_clicked();
};

#endif // PRINTERDIALOG_H
