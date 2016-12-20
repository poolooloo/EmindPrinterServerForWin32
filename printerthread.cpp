#include "printerthread.h"
//#include <QtNetwork>
#include <QPrinterInfo>

//#define PRINTER_NUM 0
#define COPIES_NUM 1
#define DMPAPER_SIZE DMPAPER_A4


PrinterThread::PrinterThread(int socketDescriptor, QObject *parent)
	:QThread(parent), socketDescriptor(socketDescriptor)
{
	totalBytes = 0;
	bytesReceived = 0;
	fileNameSize = 0;
	blockSize = 0;

}


void PrinterThread::run()
{
	qDebug() << __FUNCTION__ << endl;
	tcpSocket = new QTcpSocket();
	if (!tcpSocket->setSocketDescriptor(socketDescriptor))
	{
		emit error(tcpSocket->error());
		tcpSocket->close();
		return;
	}
	while (tcpSocket->waitForReadyRead())
	{
		QString bl = recMessage();
		QFile authfile("authcode.txt");
		if (authfile.open(QIODevice::ReadOnly))
		{
			QString authcode = "";
			authcode = QString(authfile.read(10));
			authfile.close();
			if (bl != authcode)
			{
				sendMessage("AUTH WRONG");
				return;
			}
		}
		sendMessage("OK");
		while (tcpSocket->waitForReadyRead())
		{
			QString boo = recMessage();
			qDebug() << "boo==" << boo << endl;
			if (boo != "Request printer list!")
			{
				return;
			}
			QPrinterInfo pInfo;
			QStringList pname;
			pname = pInfo.availablePrinterNames();
			pname.prepend("PList");
			qDebug() << "panme:" << pname;
			foreach(auto a, pname)
			{
				qDebug() << a;
			}
			QString pstr = pname.join(",");
			sendMessage(pstr);

			while (tcpSocket->waitForReadyRead())
			{
				recFile();
			}

		}
	}
}


void PrinterThread::sendMessage(QString messtr)
{
	qDebug() << __FUNCTION__ << endl;
	QByteArray authblock;
	QDataStream out(&authblock, QIODevice::WriteOnly);

	out << (quint16)0 <<messtr;
	out.device()->seek(0);
	out << (quint16)(authblock.size() - sizeof(quint16));
	tcpSocket->write(authblock);
	tcpSocket->flush();
}

QString PrinterThread::recMessage()
{
	qDebug() << __FUNCTION__ << endl;
	QDataStream in(tcpSocket);
	if (blockSize == 0)
	{
		qDebug() << "(blockSize == 0)" << endl;
		qDebug() << "(tcpSocket->bytesAvailable())==" << tcpSocket->bytesAvailable() << endl;
		if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
		{
			qDebug() << "return" << endl;
			return 0;
		}
		in >> blockSize;
	}
	if (tcpSocket->bytesAvailable() < blockSize)
	{
		return 0;
	}
	in >> message;
	qDebug() << "message ==" << message << endl;
	blockSize = 0;
	return message;
}


void PrinterThread::recFile()  //接收文件
{
	qDebug() << __FUNCTION__ << endl;
	while (tcpSocket->waitForReadyRead())
	{
		//qDebug() << __FUNCTION__ << endl;
		QDataStream in(tcpSocket);
		//in.setVersion(QDataStream::Qt_4_6);

		if (bytesReceived <= sizeof(qint64) * 2)
		{ //如果接收到的数据小于16个字节，那么是刚开始接收数据，我们保存到来的头文件信息

			if ((tcpSocket->bytesAvailable() >= sizeof(qint64) * 2) && (fileNameSize == 0))
			{ //接收数据总大小信息和文件名大小信息
				//in.setByteOrder(QDataStream::LittleEndian);
				in >> totalBytes >> fileNameSize;
				bytesReceived += sizeof(qint64) * 2;
				qDebug() << "bytesReceived=" << bytesReceived << endl;
			}

			if ((tcpSocket->bytesAvailable() >= fileNameSize)
				&& (fileNameSize != 0))
			{  //接收文件名，并建立文件
				in >> fileName;
				bytesReceived += fileNameSize;
				qDebug() << "bytesReceived=" << bytesReceived << endl;
				localFile = new QFile(fileName);
				if (!localFile->open(QFile::ReadWrite))
				{
					qDebug() << "open file error!";
					return;
				}
			}
			else
			{
				return;
			}
		}

		if (bytesReceived < totalBytes)
		{  //如果接收的数据小于总数据，那么写入文件
			bytesReceived += tcpSocket->bytesAvailable();
			qDebug() << "bytesReceived==" << bytesReceived << endl;
			qDebug() << "totalBytes==" << totalBytes << endl;
			inBlock = tcpSocket->readAll();
			localFile->write(inBlock);
			inBlock.resize(0);
		}

		if (bytesReceived == totalBytes)

		{ //接收数据完成时
			qDebug() << "bytesReceived == totalBytes:  Receive" << fileName << "success!" << endl;

			bytesReceived = 0;
			tcpSocket->close();
			fileNameSize = 0;
			localFile->close();
			delete localFile;

			setDefPrinter(0, fileName);
		}
	}
}


//void PrinterThread::displayError(QAbstractSocket::SocketError) //错误处理
//{
//	qDebug() << tcpSocket->errorString();
//	tcpSocket->close();
//}


void PrinterThread::setDefPrinter(int num,QString fileName1)
{
	qDebug() << __FUNCTION__ << endl;

	QPrinterInfo pInfo;
	QStringList pname;
	pname = pInfo.availablePrinterNames();
	qDebug() << "panme:" << pname;
	foreach(auto a, pname)
	{
		qDebug() << a;
	}

	int printer_num = num;
	QPrinter printer;
	QString printer_name;
	printer_name = pname.at(printer_num);
	printer.setPrinterName(printer_name);
	qDebug() << "class::printerName:" << printer.printerName() << endl;

	TCHAR szBufferDefaultPrinterName[256] = { 0 };
	memset(szBufferDefaultPrinterName, 0, 256);
	DWORD length = 256;
	GetDefaultPrinter(szBufferDefaultPrinterName, &length);
	qDebug() << "szBufferDefaultPrinterName===" << szBufferDefaultPrinterName << endl;

	//QString printerName = printer.printerName;
	//BOOL setret = FALSE;
	LPCWSTR printerName = (const wchar_t*)printer_name.utf16();
	SetDefaultPrinter(printerName);
	//SetPrinter((const wchar_t*)printer_name.utf16(),);

	/****** Set printer property! ******/

	//LONG lSize = 0;
	LPDEVMODE lpDevMode = NULL;
	HANDLE hPrinter;
	DWORD dwNeeded, dwRet;

	TCHAR defPrinter[256] = { 0 };
	memset(defPrinter, 0, 256);
	DWORD lengthDefpr = 256;
	GetDefaultPrinter(defPrinter, &lengthDefpr);
	qDebug() << "defPrinter===" << defPrinter << endl;
	//	LPDEVMODE defdevmode = getDefaultPdevmode(hPrinter);
	qDebug() << "defPrinter===" << defPrinter << endl;
	if (!OpenPrinter(defPrinter, &hPrinter, NULL))
	{
		qDebug() << "OpenPrinter==" << !OpenPrinter(defPrinter, &hPrinter, NULL) << endl;
		return;
	}

	//get real size of DEVMODE
	dwNeeded = DocumentProperties(NULL, hPrinter, defPrinter, NULL, NULL, 0);
	lpDevMode = (LPDEVMODE)malloc(dwNeeded);  //在堆中分配特定字节的空间
	dwRet = DocumentProperties(NULL, hPrinter, defPrinter, lpDevMode, NULL, DM_OUT_BUFFER);
	qDebug() << "dwRet==" << dwRet << endl;
	if (dwRet != IDOK)
	{
		free(lpDevMode);
		ClosePrinter(hPrinter);
		return;
	}
	if (lpDevMode->dmFields & DM_COPIES)
	{
		lpDevMode->dmCopies = COPIES_NUM;
		lpDevMode->dmFields |= DM_COPIES;
	}
	if (lpDevMode->dmFields & DM_ORIENTATION)
	{
		/* If the printer supports paper orientation, set it.*/
		lpDevMode->dmOrientation = DMORIENT_LANDSCAPE;  //landscape:横    portrait:纵
		lpDevMode->dmOrientation |= DM_ORIENTATION;
	}
	if (lpDevMode->dmFields & DM_PAPERSIZE)
	{
		lpDevMode->dmPaperSize = DMPAPER_SIZE;
		lpDevMode->dmOrientation |= DM_PAPERSIZE;
	}
	dwRet = DocumentProperties(NULL, hPrinter, defPrinter, lpDevMode, lpDevMode, DM_IN_BUFFER | DM_OUT_BUFFER);
	//ClosePrinter(hPrinter);
	if (dwRet != IDOK)
	{
		free(lpDevMode);
		return;
	}

	//HDC hdc = CreateDC( (LPCWSTR)(_T("winspool").AllocSysString(), printerName , NULL, lpDevMode);
	DWORD dw;
	PRINTER_INFO_2 *pi2;
	GetPrinter(hPrinter, 2, NULL, 0, &dw);
	pi2 = (PRINTER_INFO_2*)GlobalAllocPtr(GHND, dw);
	GetPrinter(hPrinter, 2, (LPBYTE)pi2, dw, &dw);

	qDebug() << "pi2->pDevMode before" << pi2->pDevMode << endl;
	qDebug() << "lpDevMode before" << lpDevMode << endl;

	pi2->pDevMode = lpDevMode;
	SetPrinter(hPrinter, 2, (LPBYTE)pi2, 0);

	QString filePath = fileName1;
	doPrint(&printer,filePath);

	ClosePrinter(hPrinter);
	GlobalFreePtr(pi2);

	_sleep(10 * 1000);
	SetDefaultPrinter(szBufferDefaultPrinterName);
	//	setDefaultPdevmode(hPrinter, defdevmode);   ////

}


void PrinterThread::doPrint(QPrinter *printer, QString fileName2)
{


	int ret = 0;

	//ShellExecute(0,
	//	QString("open").toStdWString().c_str(),
	//	QString("SumatraPDF.exe").toStdWString().c_str(),
	//	NULL,
	//	NULL,
	//	SW_HIDE);


	//执行DOS命令调系统打印
	ret = (int)ShellExecute(NULL,
		QString("print").toStdWString().c_str(),
		fileName2.toStdWString().c_str(),
		NULL,
		NULL,
		SW_HIDE);

	//    if(ret == ERROR_FILE_NOT_FOUND)
	qDebug() << "ret====" << ret << endl;

	//terminateMYSQL();
	/*qDebug() << "kill" << endl;*/

	_sleep(25 * 1000);
	BOOL booll=QFile::remove(fileName2);
	if (booll == 1)
	{
		qDebug() << "delete" << fileName2 << "success!" << endl;
	}
	else
	{
		qDebug() << "delete" << fileName2 << "failure!" << endl;
	}

}



#include <Tlhelp32.h>
void PrinterThread::terminateMYSQL()
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32* processInfo = new PROCESSENTRY32;
	processInfo->dwSize = sizeof(PROCESSENTRY32);
	int index = 0;
	int ID = 0;
	while (Process32Next(hSnapShot, processInfo) != FALSE)
	{
		index++;
		int size = WideCharToMultiByte(CP_ACP, 0, processInfo->szExeFile, -1, NULL, 0, NULL, NULL);
		char *ch = new char[size + 1];
		if (WideCharToMultiByte(CP_ACP, 0, processInfo->szExeFile, -1, ch, size, NULL, NULL))
		{
			if (strstr(ch, "SumatraPDF.exe"))
			{
				ID = processInfo->th32ProcessID;
				HANDLE hProcess;
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ID);
				if(hProcess==NULL)
				{
				  printf("Unable to get handle of process: ");
				  printf("Error is: %d",GetLastError());
				}
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
			}
		}
	}
	CloseHandle(hSnapShot);
	delete processInfo;
}


