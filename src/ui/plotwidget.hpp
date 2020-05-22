#ifndef COMPLEXPLOT_PLOTWIDGET_HPP
#define COMPLEXPLOT_PLOTWIDGET_HPP

#include <atomic>
#include <future>

#include <QWidget>
#include <QImage>

#include "engine/plotdata.hpp"

class PlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlotWidget(QWidget * parent = nullptr) : QWidget(parent) {}

    void clear(PlotData const & plotData);
    std::future<RedrawInfo> draw(PlotData const & plotData, std::atomic_bool const & cancellationToken);
    bool saveImage(QString const & path) const;

signals:
    void engineThreadExited();
    void mouseMove(QMouseEvent * event);
    void mouseLeave();

public slots:

protected:
    void paintEvent(QPaintEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void leaveEvent(QEvent * event);

private:
    QImage imageBuffer;
};

#endif // COMPLEXPLOT_PLOTWIDGET_HPP
