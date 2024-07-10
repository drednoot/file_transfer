#include "client.h"
#include <QApplication>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  View *view = new View(nullptr);

  view->show();
  return app.exec();
}
