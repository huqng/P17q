#include <Windows.h>
#include "MainWindow.h"

#include <QtWidgets/QApplication>
#include <qmessagebox.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	HANDLE hMutex = CreateMutexA(NULL, FALSE, "P17PlusSingleInstance");
	if (hMutex != nullptr && ERROR_ALREADY_EXISTS == GetLastError())
	{
		QMessageBox::warning(
			nullptr, QString::fromUtf8(u8"P17+软件已经在运行"),
			QString::fromUtf8(u8"如果上一次运行未正常结束，请在任务管理器中结束，或重启操作系统。"));

		exit(-1);
	}

    MainWindow w;
    w.show();
    return a.exec();
}
