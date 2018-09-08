#include "frame.h"

#include "board.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QLCDNumber>

Board defaultBoard() {
  Board b;
  Piece *tmp;
#define ADD_PIECE(x,y,type,side) \
  tmp = b.add_pieces(); \
  tmp->set_x(x); \
  tmp->set_y(y); \
  tmp->set_type(Piece::type); \
  tmp->set_side(side);
  
  ADD_PIECE(0,0,CHARIOT,BLACK);
  ADD_PIECE(1,0,HORSE,BLACK);
  ADD_PIECE(2,0,ELEPHANT,BLACK);
  ADD_PIECE(3,0,ADVISOR,BLACK);
  ADD_PIECE(4,0,GENERAL,BLACK);
  ADD_PIECE(5,0,ADVISOR,BLACK);
  ADD_PIECE(6,0,ELEPHANT,BLACK);
  ADD_PIECE(7,0,HORSE,BLACK);
  ADD_PIECE(8,0,CHARIOT,BLACK);

  ADD_PIECE(0,9,CHARIOT,RED);
  ADD_PIECE(1,9,HORSE,RED);
  ADD_PIECE(2,9,ELEPHANT,RED);
  ADD_PIECE(3,9,ADVISOR,RED);
  ADD_PIECE(4,9,GENERAL,RED);
  ADD_PIECE(5,9,ADVISOR,RED);
  ADD_PIECE(6,9,ELEPHANT,RED);
  ADD_PIECE(7,9,HORSE,RED);
  ADD_PIECE(8,9,CHARIOT,RED);

  ADD_PIECE(1,2,CANNON,BLACK);
  ADD_PIECE(7,2,CANNON,BLACK);

  ADD_PIECE(1,7,CANNON,RED);
  ADD_PIECE(7,7,CANNON,RED);

  ADD_PIECE(0,3,SOLDIER,BLACK);
  ADD_PIECE(2,3,SOLDIER,BLACK);
  ADD_PIECE(4,3,SOLDIER,BLACK);
  ADD_PIECE(6,3,SOLDIER,BLACK);
  ADD_PIECE(8,3,SOLDIER,BLACK);

  ADD_PIECE(0,6,SOLDIER,RED);
  ADD_PIECE(2,6,SOLDIER,RED);
  ADD_PIECE(4,6,SOLDIER,RED);
  ADD_PIECE(6,6,SOLDIER,RED);
  ADD_PIECE(8,6,SOLDIER,RED);

  return b;
}

Frame::Frame() : QMainWindow() {
  this->setWindowTitle("Chess-Chess");

  auto main = new QBoxLayout(QBoxLayout::LeftToRight);
  main->setSpacing(20);

  auto board = new BoardWidget(this);
  // TODO: connect

  main->addWidget(board);
  board->updateBoard(defaultBoard());

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