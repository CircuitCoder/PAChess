#pragma once

#include <QMainWindow>
#include <QLabel>
#include "server/server.h"
#include "board.h"


class Frame : public QMainWindow {
  Q_OBJECT

  public:
    Frame();

  private:
    Server *server = nullptr;
    BoardWidget *board;
    QLabel *status;
    Side side;
};