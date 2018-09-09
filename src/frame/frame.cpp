#include "frame.h"

#include "board.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QMenuBar>
#include <QLCDNumber>
#include <QTcpSocket>
#include <QInputDialog>

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

  board = new BoardWidget(this);
  // TODO: connect

  main->addWidget(board);

  connect(this->board, &BoardWidget::move, [this](Movement m) {
    Request req;
    *req.mutable_movement() = m;
    if(this->local) {
      if(this->server == nullptr) return;
      this->server->localApply(req);
    } else {
      this->remote->write(QByteArray::fromStdString(req.SerializeAsString()));
    }
  });

  auto sidebar = new QBoxLayout(QBoxLayout::TopToBottom);
  sidebar->setAlignment(Qt::AlignTop);
  sidebar->setSpacing(10);

  auto timer = new QLCDNumber(this);
  sidebar->addWidget(timer);

  status = new QLabel(this);
  status->setText("Connection/New Server to start a new game");
  sidebar->addWidget(status);

  auto resign = new QPushButton("Resign");
  sidebar->addWidget(resign);

  auto load = new QPushButton("Load");
  sidebar->addWidget(load);

  auto save = new QPushButton("Save");
  sidebar->addWidget(save);

  main->addLayout(sidebar, 1);

  auto container = new QWidget(this);
  container->setLayout(main);
  this->setCentralWidget(container);
  this->resize(750, 660);

  auto connMenu = menuBar()->addMenu("Connection");
  auto newServerAct = new QAction("New &Server");

  connect(newServerAct, &QAction::triggered, [this]() {
    this->local = true;
    this->board->setSide(RED);
    this->side = RED;
    this->waiting = true;
    this->waitingHint = new QMessageBox();
    this->waitingHint->setText("Waiting for the other player...");
    this->waitingHint->setStandardButtons(QMessageBox::Cancel);
    this->waitingHint->setModal(true);
    this->waitingHint->show();
    connect(this->waitingHint, &QMessageBox::buttonClicked, [this]() {
      this->server->close();
      delete this->server;
      this->server = nullptr;
    });

    this->server = new Server(5858, 60, defaultBoard());
    connect(this->server, &Server::localSync, [this](Sync s) {
      if(this->waiting) {
        this->waiting = false;
        this->waitingHint->hide();
        delete this->waitingHint;
        this->waitingHint = nullptr;
      }

      this->processSync(s);
    });
    qDebug()<<"first sync";
  });
  connMenu->addAction(newServerAct);

  auto connAct = new QAction("&Connect");
  connect(connAct, &QAction::triggered, [this]() {
    this->local = false;
    this->board->setSide(BLACK);
    this->side = BLACK;
    this->remote = new QTcpSocket(this);

    bool ok;
    auto host = QInputDialog::getText(this, "Input the host", "host", QLineEdit::Normal, "localhost", &ok);
    if(!ok) return;
    auto port = QInputDialog::getInt(this, "Input the port", "port", 5858, 0, 32768, 1, &ok);
    if(!ok) return;

    this->remote->connectToHost(host, port);
    connect(this->remote, &QTcpSocket::connected, [this]() {
      QMessageBox msg;
      msg.setText("Connected");
      msg.exec();
    });

    connect(this->remote, &QTcpSocket::readyRead, [this]() {
      qDebug()<<"Receive";
      Sync s;
      s.ParseFromString(this->remote->readAll().toStdString());
      this->processSync(s);
    });
  });
  connMenu->addAction(connAct);
}

void Frame::processSync(Sync s) {
  if(s.has_call()) ; // TODO: process call
  else if(s.has_board()) {
    this->board->updateBoard(s.board());
    qDebug()<<"Board";
  } else {
    this->board->setMovable(this->side == s.side());
    if(s.side() == RED) this->status->setText("Red moves");
    else this->status->setText("Black moves");
  }
}