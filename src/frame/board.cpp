#include "board.h"

#include <QImage>
#include <QPixmap>
#include <QtSvg>
#include <QLabel>

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
  // TODO: impl
}

void BoardWidget::clearBoard() {
  // TODO: impl
}