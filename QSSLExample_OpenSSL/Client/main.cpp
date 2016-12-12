#include <QtGui/QApplication>
#include "StdAfx.h"
#include "client.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  Client SSLClient;
  StartCounter();
  if (argc < 3)
  {
	  std::cout << "Usage : SSLClient [host] [port]" << std::endl;
	  std::cout << "Example : SSLClient 127.0.0.1 8081" << std::endl;
  }
  QString HostName = "127.0.0.1";
  int Port = 8081;
  if (argc > 1)
	  HostName = argv[1];
  if (argc > 2)
	  Port = atoi( argv[2] );

  SSLClient.ToggleConnectDisconnect(HostName, Port);

  int ExecRet = a.exec();

  return ExecRet;
}
