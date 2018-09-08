#pragma once

#include <QWidget>
#include <QTcpServer>
#include <QTimer>
#include <vector>
#include <shared_mutex>
#include "proto/msg.pb.h"

class Server : QWidget {
  Q_OBJECT

  public:
    Server(quint16 port, int timeout = 60);
    Response localApply(Request req);

  signals:
    void localSync(Sync sync);

  private:
    QTcpServer *server;
    QTimer *timer;
    std::vector<Piece> board;
    Side localSide;
    Side currentSide;
    bool started = false;

    std::shared_mutex boardMutex;

    Response apply(Request req, Side side);
    void syncBoard();
    void syncSide();
    void call(Call call);
};
