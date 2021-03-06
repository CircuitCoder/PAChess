#include "server.h"

#include <QTcpSocket>

using namespace std;

Side negateSide(Side s) {
  if(s == RED) return BLACK;
  else return RED;
}

Server::Server(quint16 port, int timeout, Board init) {
  this->server = new QTcpServer(this);
  this->server->listen(QHostAddress::Any, port);

  this->timer = new QTimer(this);
  this->timer->setInterval(timeout * 1000);
  connect(this->timer, &QTimer::timeout, [this]() {
    Sync sync;
    Call call;
    call.set_winner(negateSide(this->currentSide));
    call.set_cause(Call::TIMEOUT);
    *sync.mutable_call() = call;
    
    emit localSync(sync);
    this->remoteSync(sync);
  });

  this->localSide = RED;
  this->currentSide = RED;

  for(int i = 0; i<init.pieces_size(); ++i)
    board.push_back(init.pieces(i));
  
  connect(this->server, &QTcpServer::newConnection, [this]() {
    this->remote = this->server->nextPendingConnection();

    connect(this->remote, &QTcpSocket::readyRead, [this]() {
      while(true) {
        if(this->remote->QIODevice::bytesAvailable() < 2) return;
        unsigned char high, low;
        this->remote->getChar((char*) &low);
        this->remote->getChar((char*) &high);
        int count = ((int) high) << 8 | low;
        if(this->remote->bytesAvailable() < count) {
          this->remote->ungetChar(high);
          this->remote->ungetChar(low);
          return;
        }

        qDebug()<<"RECV:"<<count;

        auto data = this->remote->QIODevice::read(count);
        Request req;
        req.ParseFromString(data.toStdString());
        this->apply(req, negateSide(this->localSide));
      }
    });

    this->syncBoard();
    this->syncSide();

    this->timer->start();
  });
}

Response Server::localApply(Request req) {
  return this->apply(req, this->localSide);
}

Response Server::apply(Request req, Side side) {
  if(req.has_resign()) {
    Call call;
    call.set_winner(negateSide(side));
    call.set_cause(Call::RESIGN);

    this->call(call);

    Response resp;
    resp.set_success(true);
    return resp;
  } else if(req.has_upload()) {
    Board init = req.upload();
    board.clear();
    for(int i = 0; i<init.pieces_size(); ++i)
      board.push_back(init.pieces(i));
    this->syncBoard();
    Response resp;
    resp.set_success(true);
    return resp;
  } else {
    // Movement
    const auto movement = req.movement();

    Response resp;

    unique_lock<shared_mutex> lock(boardMutex);
    // Search through the board
    // TODO: validation
    for(auto i = this->board.begin(); i != this->board.end(); ++i)
      if(i->x() == movement.to_x() && i->y() == movement.to_y()) {
        const auto removed = *i;
        this->board.erase(i);
        if(removed.type() == Piece::GENERAL) {
          Call call;
          call.set_winner(negateSide(removed.side()));
          call.set_cause(Call::CHECKMATE);

          this->call(call);
        }
        break;
      }

    for(auto i = this->board.begin(); i != this->board.end(); ++i)
      if(i->x() == movement.from_x() && i->y() == movement.from_y()) {
        i->set_x(movement.to_x());
        i->set_y(movement.to_y());
      }
    
    lock.unlock();
    
    this->syncBoard();

    this->currentSide = negateSide(this->currentSide);
    this->syncSide();

    this->timer->stop();
    this->timer->start();

    resp.set_success(true);
    return resp;
  }
}

void Server::syncBoard() {
  Sync sync;
  shared_lock<shared_mutex> lock(boardMutex);
  Board *b = sync.mutable_board();

  for(const auto &piece : this->board)
    *b->add_pieces() = piece;
  
  emit localSync(sync);
  this->remoteSync(sync);
}

void Server::call(Call call) {
  Sync sync;
  *sync.mutable_call() = call;
  emit localSync(sync);
  this->remoteSync(sync);
}

void Server::syncSide() {
  Sync sync;
  sync.set_side(this->currentSide);
  
  emit localSync(sync);
  this->remoteSync(sync);
}

void Server::remoteSync(Sync sync) {
  if(!this->remote) return;
  auto array = QByteArray::fromStdString(sync.SerializeAsString());
  short len = array.length();
  qDebug()<<"SEND:"<<len;
  unique_lock<shared_mutex> lock(this->networkMutex);
  this->remote->putChar(len & 0xFF);
  this->remote->putChar(len >> 8);
  qDebug()<<"SPLIT:"<<(len & 0xFF)<<(len>>8);
  this->remote->write(array);
}
