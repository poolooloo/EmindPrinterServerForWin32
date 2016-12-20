#ifndef PRINTERSERVER_H
#define PRINTERSERVER_H

#include <QTcpServer>
#include <qstring.h>

class PrinterServer : public QTcpServer
{
	Q_OBJECT
public:
	PrinterServer(QObject *parent = 0);
protected:
	void incomingConnection(qintptr socketDescriptor);

};

#endif // PRINTERTHREAD_H

