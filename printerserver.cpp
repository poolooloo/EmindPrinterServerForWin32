#include "printerserver.h"
#include "printerthread.h"
#include <qprinterinfo.h>

PrinterServer::PrinterServer(QObject *parent)
	:QTcpServer(parent)
{
	//dlg = (PrinterDialog *)parent;
}

void PrinterServer::incomingConnection(qintptr socketDescriptor)
{
	qDebug() << __FUNCTION__ << endl;
	PrinterThread *thread = new PrinterThread(socketDescriptor, this);
	qDebug() << "Connect server Success!";
	connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
	thread->start();
	qDebug() << "Server connection socketDescriptor==" << socketDescriptor;
	
}

