#include <QApplication>
#include "frame/frame.h"

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  Frame frame;
  frame.show();

  return app.exec();
}
