#ifndef PRINTERTHREAD_H
#define PRINTERTHREAD_H

#include <QThread>
#include <QtNetwork>
#include <QTcpSocket>
#include <QPrinter>
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>


class PrinterThread : public QThread
{
	Q_OBJECT
public:
	PrinterThread(int socketDescriptor, QObject *parent = 0);
	void run() ;
	void sendMessage(QString messtr);
	QString recMessage();
	void recFile();
	void setDefPrinter(int num, QString fileName1);
	void doPrint(QPrinter *printer, QString fileName2);
	void terminateMYSQL();
signals:
	void error(QTcpSocket::SocketError socketError);

private:
	int socketDescriptor;
	QTcpServer tcpServer;
	QTcpSocket *tcpSocket;
	qint64 totalBytes;
	qint64 bytesReceived;
	qint64 fileNameSize;
	QString fileName;
	QFile *localFile;
	QByteArray inBlock;
	quint16 blockSize;
	QString message;

};

#endif // PRINTERTHREAD_H
