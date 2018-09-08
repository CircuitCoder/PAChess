#pragma once

#include <QWidget>
#include <QLabel>
#include <vector>
#include <optional>
#include <QMouseEvent>
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
    void setMovable(bool movable);
    void setSide(Side side);

  private:
    Board b;
    int moving = -1;

    bool movable = true;

    Side side = RED;

    void updateWidgets();
    std::vector<std::pair<int, int>> query(Piece p);
    bool freeAt(int x,int y);
    bool enemyAt(int x, int y);
    bool selfAt(int x, int y);
    std::optional<Side> sideAt(int x, int y);
    void initMove(int i, std::pair<int, int>);

    std::vector<QLabel *> pieces;
};

class Clickable : public QLabel {
  Q_OBJECT

  public:
    Clickable(QWidget *parent) : QLabel(parent) { }

  signals:
    void clicked();

  protected:
    inline
    void mousePressEvent(QMouseEvent *e) {
      if(e->button() == Qt::LeftButton)
        emit clicked();
    }
};