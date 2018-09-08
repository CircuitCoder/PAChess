#pragma once

#include <QWidget>
#include "proto/msg.pb.h"

class BoardWidget : public QWidget {
  Q_OBJECT

  public:
    BoardWidget(QWidget *parent = nullptr);

  signals:
    void move(Movement movement);

  public slots:
    void updateBoard(Board board);
    void clearBoard();

  private:
    Board b;
};
