#include "server.h"
#include <QApplication>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  Server *server = new Server();
  (void)server;

  return app.exec();
}
