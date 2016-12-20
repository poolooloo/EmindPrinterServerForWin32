//#include <winspool.h>
#include "printerdialog.h"
#include "ui_printerdialog.h"
#include <QDebug>
#include <QTimer>
#include <QNetworkInterface>
#include <qfile.h>

//#include <QTextStream>
//#include <QFile>

//#include <poppler-qt5.h>
//#include <iostream>


PrinterDialog::PrinterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrinterDialog)
{
	qDebug() << __FUNCTION__ << endl;
    ui->setupUi(this);
    Qt::WindowFlags flags=Qt::Dialog;
 //        flags |= Qt::CustomizeWindowHint;
    flags |= Qt::WindowMinimizeButtonHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

	QFile authfile("authcode.txt");
	if (authfile.open(QIODevice::ReadOnly))
	{
		QString authcode = "";
		authcode = QString(authfile.read(10));
		authfile.close();
		qDebug() << "authcode" << authcode << endl;
		ui->authleEdit->setText(authcode);
	 }

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(getIP()));
    timer->start(200);
	
	tcpPort = 6666;
	//printerServer = new PrinterServer(this);
	if (!printerServer.listen(QHostAddress::Any, tcpPort))
	{
		qDebug() << printerServer.errorString();
		close();
		return;
	}
}


PrinterDialog::~PrinterDialog()
{
    delete ui;
}

void PrinterDialog::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       if(address.protocol() == QAbstractSocket::IPv4Protocol)
       {
           if (address.toString().contains("127.0."))
           {
               continue;
           }
           ui->ipleEdit->setText(address.toString());
           return;
       }
       else
       {
           ui->ipleEdit->setText("");
       }
    }
}

void PrinterDialog::on_flushButton_clicked()
{
	QFile file("authcode.txt");

	if (file.open(QIODevice::WriteOnly)) 
	{
		QString authcode = ui->authleEdit->text();
		file.write(authcode.toStdString().c_str()); 
		file.close();
	}
}



