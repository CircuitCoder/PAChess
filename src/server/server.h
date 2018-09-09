#pragma once

#include <QWidget>
#include <QTcpServer>
#include <QTimer>
#include <vector>
#include <shared_mutex>
#include "proto/msg.pb.h"

class Server : public QWidget {
  Q_OBJECT

  public:
    Server(quint16 port, int timeout = 60, Board init = Board());
    Response localApply(Request req);
    void syncBoard();
    void syncSide();

  signals:
    void localSync(Sync sync);

  private:
    QTcpServer *server;
    QTcpSocket *remote = nullptr;
    QTimer *timer;
    std::vector<Piece> board;
    Side localSide;
    Side currentSide;
    bool started = false;

    std::shared_mutex boardMutex;
    std::shared_mutex networkMutex;

    Response apply(Request req, Side side);
    void call(Call call);
    void remoteSync(Sync sync);
};
