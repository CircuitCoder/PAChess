#include "board.h"

#include <QImage>
#include <QPixmap>
#include <QtSvg>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>

using namespace std;


QString pieceName(Piece::Type type) {
  if(type == Piece::GENERAL) return "general";
  if(type == Piece::ADVISOR) return "advisor";
  if(type == Piece::ELEPHANT) return "elephant";
  if(type == Piece::HORSE) return "horse";
  if(type == Piece::CHARIOT) return "chariot";
  if(type == Piece::CANNON) return "cannon";
  return "soldier";
}

QImage pieceImage(Piece::Type type, Side side) {
  auto name = pieceName(type);
  QImage image(QString("./assets/pieces/%1.png").arg(name));
  if(side == RED) return image.copy(0, 0, 64, 64);
  else return image.copy(132 - 64, 0, 64, 64);
}

BoardWidget::BoardWidget(QWidget *parent) : QWidget(parent) {
  this->setFixedSize(600, 660);
  QImage board(600, 660, QImage::Format_ARGB32);
  QSvgRenderer ren(QString("./assets/board.svg"));
  QPainter painter(&board);
  ren.render(&painter);

  auto boardLabel = new QLabel(this);
  QPixmap boardPix;
  boardPix.convertFromImage(board);
  boardLabel->setPixmap(boardPix);
  boardLabel->show();
}

void BoardWidget::updateBoard(Board board) {
  this->b = board;
  this->updateWidgets();
}

void BoardWidget::clearBoard() {
  this->updateBoard(Board());
}

void BoardWidget::setMovable(bool m) {
  this->movable = m;
  this->updateWidgets();
}

void BoardWidget::setSide(Side s) {
  this->side = s;
  this->updateWidgets();
}

void BoardWidget::updateWidgets() {
  for(auto label : this->pieces) {
    label->hide();
    delete label;
  }

  this->pieces.clear();
  for(int i = 0; i<this->b.pieces_size(); ++i) {
    auto l = new Clickable(this);
    auto piece = this->b.pieces(i);
    QPixmap pm;
    pm.convertFromImage(pieceImage(piece.type(), piece.side()));
    l->setPixmap(pm);
    l->setFixedSize(64, 64);
    l->setGeometry(
      (600 - 480) / 2 - 32 + piece.x() * 60,
      (660 - 540) / 2 - 32 + piece.y() * 60,
      64,
      64);
    
    if(piece.side() == this->side && this->movable) {
      l->setCursor(Qt::CursorShape::PointingHandCursor);
      connect(l, &Clickable::clicked, [this, i]() {
        qDebug()<<"Click";
        this->moving = i;
        this->updateWidgets();
      });
    }
    
    l->show();
    this->pieces.push_back(l);
  }

  if(this->moving != -1) {
    qDebug()<<"Query";
    auto piece = this->b.pieces(this->moving);
    auto pos = query(piece);
    qDebug()<<pos;
    for(auto p : pos) {
      auto &[x, y] = p;
      auto l = new Clickable(this);
      QPixmap pm;
      auto img = pieceImage(piece.type(), piece.side());
      QImage newImg(img.size(), img.format());
      newImg.fill(qRgba(0,0,0,0));
      QPainter pt(&newImg);
      pt.setOpacity(0.3);
      pt.drawImage(QRect(0, 0, img.width(), img.height()), img);

      pm.convertFromImage(newImg);
      l->setPixmap(pm);
      l->setFixedSize(64, 64);
      l->setGeometry(
        (600 - 480) / 2 - 32 + x * 60,
        (660 - 540) / 2 - 32 + y * 60,
        64,
        64);

      l->setCursor(Qt::CursorShape::PointingHandCursor);
      connect(l, &Clickable::clicked, [this, p]() {
        this->initMove(this->moving, p);
      });

      l->setStyleSheet("opacity: 0.3");
      l->show();
      this->pieces.push_back(l);
    }
  }
}

vector<pair<int, int>> BoardWidget::query(Piece p) {
  vector<pair<int, int>> result;
  if(p.type() == Piece::GENERAL) {
    int x = p.x();
    int y = p.y();

    int ylb = p.side() == BLACK ? 0 : 7;
    int yub = p.side() == BLACK ? 2 : 9;

    if(x > 3 && !selfAt(x-1, y)) result.push_back({ x -1, y });
    if(x < 5 && !selfAt(x+1, y)) result.push_back({ x -1, y });
    if(y > ylb && !selfAt(x, y-1)) result.push_back({ x, y -1 });
    if(y < yub && !selfAt(x, y+1)) result.push_back({ x, y +1 });
  } else if(p.type() == Piece::ADVISOR) {
    int x = p.x();
    int y = p.y();
    int ylb = p.side() == BLACK ? 0 : 2;
    int yub = p.side() == BLACK ? 7 : 9;
    if(x > 3) {
      if(y > ylb && !selfAt(x-1, y-1)) result.push_back({ x-1, y-1 });
      if(y < yub && !selfAt(x-1, y+1)) result.push_back({ x-1, y+1 });
    }

    if(x < 5) {
      if(y > ylb && !selfAt(x+1, y-1)) result.push_back({ x+1, y-1 });
      if(y < yub && !selfAt(x+1, y+1)) result.push_back({ x+1, y+1 });
    }
  } else if(p.type() == Piece::ELEPHANT) {
    int x = p.x();
    int y = p.y();
    int ylb = p.side() == BLACK ? 0 : 5;
    int yub = p.side() == BLACK ? 4 : 9;

    if(x > 0) {
      if(y > ylb && freeAt(x-1, y-1) && !selfAt(x-2, y-2)) result.push_back({ x-2, y-2 });
      if(y < yub && freeAt(x-1, y+1) && !selfAt(x-2, y+2)) result.push_back({ x-2, y+2 });
    }

    if(x < 8) {
      if(y > ylb && freeAt(x+1, y-1) && !selfAt(x+2, y-2)) result.push_back({ x+2, y-2 });
      if(y < yub && freeAt(x+1, y+1) && !selfAt(x+2, y+2)) result.push_back({ x+2, y+2 });
    }
  } else if(p.type() == Piece::HORSE) {
    int x = p.x();
    int y = p.y();

    vector<pair<int, int>> stash;

    if(freeAt(x, y-1)) {
      if(!selfAt(x-1, y-2)) stash.push_back({ x-1, y-2 });
      if(!selfAt(x+1, y-2)) stash.push_back({ x+1, y-2 });
    }

    if(freeAt(x, y+1)) {
      if(!selfAt(x-1, y+2)) stash.push_back({ x-1, y+2 });
      if(!selfAt(x+1, y+2)) stash.push_back({ x+1, y+2 });
    }

    if(freeAt(x-1, y)) {
      if(!selfAt(x-2, y-1)) stash.push_back({ x-2, y-1 });
      if(!selfAt(x-2, y+1)) stash.push_back({ x-2, y+1 });
    }

    if(freeAt(x+1, y)) {
      if(!selfAt(x+2, y-1)) stash.push_back({ x+2, y-1 });
      if(!selfAt(x+2, y+1)) stash.push_back({ x+2, y+1 });
    }

    for(auto &i : stash)
      if(i.first <= 8 && i.first >= 0 && i.second <= 9 && i.second >= 0)
        result.push_back(i);
  } else if(p.type() == Piece::CHARIOT) {
    int x = p.x();
    int y = p.y();
    for(int i = x+1; i <= 8; ++i) {
      if(selfAt(i, y)) break;
      result.push_back({ i, y });
      if(enemyAt(i, y)) break;
    }

    for(int i = x-1; i >= 0; --i) {
      if(selfAt(i, y)) break;
      result.push_back({ i, y });
      if(enemyAt(i, y)) break;
    }

    for(int i = y+1; i <= 9; ++i) {
      if(selfAt(x, i)) break;
      result.push_back({ x, i });
      if(enemyAt(x, i)) break;
    }

    for(int i = y-1; i >= 0; --i) {
      if(selfAt(x, i)) break;
      result.push_back({ x, i });
      if(enemyAt(x, i)) break;
    }
  } else if(p.type() == Piece::CANNON) {
    int x = p.x();
    int y = p.y();
    for(int i = x+1; i <= 8; ++i)
      if(!freeAt(i, y)) {
        for(int j = i+1; j <= 8; ++j)
          if(enemyAt(j, y)) {
            result.push_back({ j, y });
            break;
          } else if(selfAt(j, y)) break;
        break;
      }

    for(int i = x-1; i >= 0; --i)
      if(!freeAt(i, y)) {
        for(int j = i-1; j >= 0; --j)
          if(enemyAt(j, y)) {
            result.push_back({ j, y });
            break;
          } else if(selfAt(j, y)) break;
        break;
      }

    for(int i = y+1; i <= 9; ++i)
      if(!freeAt(x, i)) {
        for(int j = i+1; j <= 0; ++j)
          if(enemyAt(x, j)) {
            result.push_back({ x, j });
            break;
          } else if(selfAt(x, j)) break;
        break;
      }

    for(int i = y-1; i >= 0; --i)
      if(!freeAt(x, i)) {
        for(int j = i-1; j >= 0; --j)
          if(enemyAt(x, j)) {
            result.push_back({ x, j });
            break;
          } else if(selfAt(x, j)) break;
        break;
      }
  } else {
    int x = p.x();
    int y = p.y();

    bool overriver = p.side() == BLACK ? y > 4 : y < 5;

    if(p.side() == BLACK && y < 9 && !selfAt(x, y+1)) result.push_back({ x, y+1 });
    if(p.side() == RED && y > 0 && !selfAt(x, y-1)) result.push_back({ x, y-1 });

    if(overriver) {
      if(x > 0 && !selfAt(x-1, y)) result.push_back({ x-1, y });
      if(x < 8 && !selfAt(x+1, y)) result.push_back({ x+1, y });
    }
  }

  return result;
}

bool BoardWidget::freeAt(int x, int y) {
  return !sideAt(x, y).has_value();
}

bool BoardWidget::selfAt(int x, int y) {
  auto s = sideAt(x, y);
  return s && *s == this->side;
}

bool BoardWidget::enemyAt(int x, int y) {
  auto s = sideAt(x, y);
  return s && *s != this->side;
}

optional<Side> BoardWidget::sideAt(int x, int y) {
  for(int i = 0; i<this->b.pieces_size(); ++i) {
    auto p = b.pieces(i);
    if(p.x() == x && p.y() == y) return { p.side() };
  }

  return {};
}

void BoardWidget::initMove(int i, pair<int, int> to) {
  auto p = b.pieces(i);
  Movement m;
  m.set_from_x(p.x());
  m.set_from_y(p.y());
  m.set_to_x(to.first);
  m.set_to_y(to.second);

  emit move(m);
}