#include "frame.h"

#include "board.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLCDNumber>

Frame::Frame() : QMainWindow() {
  this->setWindowTitle("Chess-Chess");

  auto main = new QBoxLayout(QBoxLayout::LeftToRight);
  main->setSpacing(20);

  auto board = new BoardWidget(this);
  // TODO: connect

  main->addWidget(board);

  auto sidebar = new QBoxLayout(QBoxLayout::TopToBottom);
  sidebar->setAlignment(Qt::AlignTop);
  sidebar->setSpacing(10);

  auto timer = new QLCDNumber(this);
  sidebar->addWidget(timer);

  auto resign = new QPushButton("Resign");
  sidebar->addWidget(resign);

  auto load = new QPushButton("Load");
  sidebar->addWidget(load);

  main->addLayout(sidebar, 1);

  auto container = new QWidget(this);
  container->setLayout(main);
  this->setCentralWidget(container);
  this->resize(750, 660);
}