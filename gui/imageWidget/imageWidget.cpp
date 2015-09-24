#include <QtCore>
#include "imageWidget.h"

ImageWidget::ImageWidget(QWidget *parent)
	: QWidget(parent)
{
    mPixmap = QPixmap();
}

void
ImageWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap fitPixmap = mPixmap.scaled(width(),height(), Qt::KeepAspectRatio);
    painter.drawPixmap(0, 0, fitPixmap);
}

void
ImageWidget::setPixmap(QString fileName)
{
    mPixmap.load(fileName);
    update();
}

QPixmap
ImageWidget::getPixmap()
{
    return mPixmap;
}



