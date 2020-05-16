#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <chrono>

#include <QWidget>
#include <QImage>

#include "plotdata.h"

struct RedrawInfo
{
    using DurationType = std::chrono::duration<double>;

    DurationType parsingDuration;
    DurationType computingDuration;
    DurationType coloringDuration;
};

class PlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlotWidget(QWidget * parent = nullptr) : QWidget(parent) {}

    void redraw(PlotData const & plotData, RedrawInfo & info);
    bool saveImage(QString const & path) const;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent * event);

private:
    QImage imageBuffer;
};

#endif // PLOTWIDGET_H
