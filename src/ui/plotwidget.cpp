#include <future>

#include <QColor>
#include <QPainter>
#include <QString>

#include "engine/engine.hpp"
#include "ui/plotwidget.hpp"

std::future<RedrawInfo> PlotWidget::draw(PlotData plotData)
{
    imageBuffer = QImage(plotData.imageWidth, plotData.imageHeight, QImage::Format_RGB888);
    QColor blank(64, 64, 64);
    imageBuffer.fill(blank);
    setFixedSize(plotData.imageWidth, plotData.imageHeight);
    repaint();

    auto update = [this](int x, int y, double r, double g, double b)
    {
        imageBuffer.setPixelColor(x, y, QColor(int(r*255.9), int(g*255.9), int(b*255.9)));
    };

    return std::async(&redraw<decltype(update)>, std::move(plotData), std::move(update));
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
