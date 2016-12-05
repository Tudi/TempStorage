#include <QtGui/QApplication>
#include "client.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  Client w;

  w.connectDisconnectButtonPressed();
  w.sendButtonPressed();

  return a.exec();
}
