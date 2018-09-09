#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QMessageBox>
#include "server/server.h"
#include "board.h"


class Frame : public QMainWindow {
  Q_OBJECT

  public:
    Frame();

  private:
    Server *server = nullptr;
    QTcpSocket *remote = nullptr;
    BoardWidget *board;
    QLabel *status;
    Side side;
    bool local;
    bool waiting = false;
    QMessageBox *waitingHint;

    void processSync(Sync sync);
};