#ifndef IMAGEWIDGET_H_
#define IMAGEWIDGET_H_

#include <QtGui>

class ImageWidget : public QWidget
{
	Q_OBJECT

public:
	ImageWidget(QWidget *parent = 0);
	void setPixmap(QString fileName);
	QPixmap getPixmap();

protected:
        void paintEvent(QPaintEvent *event);

private:
        QPixmap mPixmap;
};

#endif /*IMAGEWIDGET_H_*/
