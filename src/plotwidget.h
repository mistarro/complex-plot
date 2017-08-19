#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QImage>

#include "plotdata.h"

class PlotWidget : public QWidget
{
  Q_OBJECT
public:
  explicit PlotWidget(QWidget * parent = nullptr) : QWidget(parent) {}

  void redraw(PlotData const & plotData);

signals:

public slots:

protected:
  void paintEvent(QPaintEvent * event);

private:
  QImage imageBuffer;
};

#endif // PLOTWIDGET_H
