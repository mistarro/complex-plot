#ifndef COMPLEXPLOT_PLOTWIDGET_HPP
#define COMPLEXPLOT_PLOTWIDGET_HPP

#include <future>

#include <QWidget>
#include <QImage>

#include "engine/plotdata.hpp"

class PlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlotWidget(QWidget * parent = nullptr) : QWidget(parent) {}

    std::future<RedrawInfo> draw(PlotData plotData);
    bool saveImage(QString const & path) const;

signals:

public slots:

protected:
    void paintEvent(QPaintEvent * event);

private:
    QImage imageBuffer;
};

#endif // COMPLEXPLOT_PLOTWIDGET_HPP
