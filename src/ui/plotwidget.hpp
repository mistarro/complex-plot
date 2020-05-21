#ifndef COMPLEXPLOT_PLOTWIDGET_HPP
#define COMPLEXPLOT_PLOTWIDGET_HPP

#include <QWidget>
#include <QImage>

#include "engine/plotdata.hpp"

class PlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlotWidget(QWidget * parent = nullptr) : QWidget(parent) {}

    void draw(PlotData const & plotData, RedrawInfo & info);
    bool saveImage(QString const & path) const;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent * event);

private:
    QImage imageBuffer;
};

#endif // COMPLEXPLOT_PLOTWIDGET_HPP
