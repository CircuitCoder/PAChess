#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QLCDNumber>
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
    QTimer *timer;
    QLCDNumber *timerDisplay;
    int left = 60;

    void processSync(Sync sync);
};