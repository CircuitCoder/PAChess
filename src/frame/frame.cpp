#include "frame.h"

#include "board.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QMenuBar>
#include <QTcpSocket>
#include <QInputDialog>
#include <QFileDialog>
#include <iostream>
#include <fstream>

using namespace std;

void filterOutput(ostream &s, Board &b, Side side, Piece::Type type) {
  int counter = 0;
  for(int i = 0; i<b.pieces_size(); ++i) {
    auto piece = b.pieces(i);
    if(piece.side() == side && piece.type() == type) ++counter;
  }

  s<<counter<<" ";
  for(int i = 0; i<b.pieces_size(); ++i) {
    auto piece = b.pieces(i);
    if(piece.side() == side && piece.type() == type) {
      --counter;
      s<<"<"<<piece.x()<<","<<9-piece.y()<<">";
      if(counter != 0) s<<" ";
      else s<<endl;
    }
  }
}

ostream& operator<<(ostream &s, Board b) {
  s<<"red"<<endl;
  filterOutput(s, b, RED, Piece::GENERAL);
  filterOutput(s, b, RED, Piece::ADVISOR);
  filterOutput(s, b, RED, Piece::ELEPHANT);
  filterOutput(s, b, RED, Piece::HORSE);
  filterOutput(s, b, RED, Piece::CHARIOT);
  filterOutput(s, b, RED, Piece::CANNON);
  filterOutput(s, b, RED, Piece::SOLDIER);
  s<<"black"<<endl;
  filterOutput(s, b, BLACK, Piece::GENERAL);
  filterOutput(s, b, BLACK, Piece::ADVISOR);
  filterOutput(s, b, BLACK, Piece::ELEPHANT);
  filterOutput(s, b, BLACK, Piece::HORSE);
  filterOutput(s, b, BLACK, Piece::CHARIOT);
  filterOutput(s, b, BLACK, Piece::CANNON);
  filterOutput(s, b, BLACK, Piece::SOLDIER);
  return s;
}

void filterInput(istream &s, Board &b, Side side, Piece::Type type) {
  int count;
  s>>count;
  for(int i = 0; i<count; ++i) {
    string buf;
    s>>buf;
    int x = buf[1] - '0';
    int y = buf[3] - '0';

    auto p = b.add_pieces();
    p->set_x(x);
    p->set_y(9-y);
    p->set_side(side);
    p->set_type(type);
  }
}

istream& operator>>(istream &s, Board &b) {
  for(int i = 0; i<2; ++i) {
    string color;
    s>>color;
    Side side = color == "red" ? RED : BLACK;

    filterInput(s, b, side, Piece::GENERAL);
    filterInput(s, b, side, Piece::ADVISOR);
    filterInput(s, b, side, Piece::ELEPHANT);
    filterInput(s, b, side, Piece::HORSE);
    filterInput(s, b, side, Piece::CHARIOT);
    filterInput(s, b, side, Piece::CANNON);
    filterInput(s, b, side, Piece::SOLDIER);
  }
  return s;
}

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

  this->timer = new QTimer(this);
  this->timer->setInterval(1000);

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

  timerDisplay = new QLCDNumber(this);
  sidebar->addWidget(timerDisplay);

  connect(this->timer, &QTimer::timeout, [this]() {
    this->timerDisplay->display(--this->left);
  });

  status = new QLabel(this);
  status->setText("Connection/New Server to start a new game");
  sidebar->addWidget(status);

  auto resign = new QPushButton("Resign");
  connect(resign, &QPushButton::clicked, [this]() {
    Request req;
    *req.mutable_resign() = Resign();
    if(this->local) {
      if(this->server == nullptr) return;
      this->server->localApply(req);
    } else {
      this->remote->write(QByteArray::fromStdString(req.SerializeAsString()));
    }
  });
  sidebar->addWidget(resign);

  auto load = new QPushButton("Load");
  sidebar->addWidget(load);
  connect(load , &QPushButton::clicked, [this]() {
    auto fn = QFileDialog::getOpenFileName(this, "Open Save...", "save.txt");
    ifstream fis(fn.toStdString());

    Request req;
    fis>>*req.mutable_upload();
    if(this->local) {
      if(this->server == nullptr) return;
      this->server->localApply(req);
    } else {
      this->remote->write(QByteArray::fromStdString(req.SerializeAsString()));
    }
  });

  auto save = new QPushButton("Save");
  sidebar->addWidget(save);
  connect(save, &QPushButton::clicked, [this]() {
    auto fn = QFileDialog::getSaveFileName(this, "Save To...", "save.txt");
    ofstream fos(fn.toStdString());
    fos<<this->board->getBoard();
  });

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
  if(s.has_call()) {
    auto call = s.call();
    this->timer->stop();

    auto msg = new QMessageBox();

    if(call.winner() == this->side) {
      if(call.cause() == Call::RESIGN)
        msg->setText("You WIN! Your opposite resigned!");
      else if(call.cause() == Call::CHECKMATE)
        msg->setText("You WIN! Your opposite has been checkmate!");
      else
        msg->setText("You WIN! Your opposite timed out!");

      this->status->setText("You win");
    } else {
      if(call.cause() == Call::RESIGN)
        msg->setText("You LOSE! You resigned!");
      else if(call.cause() == Call::CHECKMATE)
        msg->setText("You LOSE! You have been checkmate!");
      else
        msg->setText("You LOSE! You timed out!");

      this->status->setText("You lose");
    }
    msg->setModal(true);
    msg->show();

    connect(msg, &QMessageBox::buttonClicked, [msg]() {
      delete msg;
    });

    if(this->local) this->server->close();
    else this->remote->close();

    this->board->setMovable(false);
  } else if(s.has_board()) {
    this->board->updateBoard(s.board());
  } else {
    this->board->setMovable(this->side == s.side());
    if(s.side() == RED) this->status->setText("Red moves");
    else this->status->setText("Black moves");

    if(this->side == s.side()) {
      this->left = 60;
      this->timerDisplay->display(60);
      this->timer->start();
    } else {
      this->left = 60;
      this->timerDisplay->display(9999);
      this->timer->stop();
    }
  }
}