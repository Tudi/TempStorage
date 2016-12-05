#include <QtGui/QApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server w;

	w.checkFileStatus();
	w.startStopButtonClicked();

    return a.exec();
}
