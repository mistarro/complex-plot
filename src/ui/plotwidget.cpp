#include <functional>
#include <future>

#include <QColor>
#include <QPainter>
#include <QString>

#include "engine/engine.hpp"
#include "ui/plotwidget.hpp"

void PlotWidget::clear(PlotData const & plotData)
{
    imageBuffer = QImage(plotData.imageWidth, plotData.imageHeight, QImage::Format_RGB888);
    setFixedSize(plotData.imageWidth, plotData.imageHeight);
    QColor blank(64, 64, 64);
    imageBuffer.fill(blank);
    repaint();
}

std::future<RedrawInfo> PlotWidget::draw(PlotData const & plotData, std::atomic_bool const & cancellationToken)
{
    clear(plotData);

    auto update = [this](int x, int y, double r, double g, double b)
    {
        imageBuffer.setPixelColor(x, y, QColor(int(r*255.9), int(g*255.9), int(b*255.9)));
    };

    auto notifyExit = [this]()
    {
        emit engineThreadExited();
    };

    return std::async(&redraw<decltype(update), decltype(notifyExit)>,
                      std::ref(plotData),
                      std::move(update),
                      std::move(notifyExit),
                      std::ref(cancellationToken));
}

bool PlotWidget::saveImage(QString const & path) const
{
    return imageBuffer.save(path);
}

void PlotWidget::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.drawImage(0, 0, imageBuffer);
}

void PlotWidget::mouseMoveEvent(QMouseEvent * event)
{
    emit mouseMove(event);
}

void PlotWidget::leaveEvent(QEvent * event)
{
    emit mouseLeave();
}
