#include <QColor>
#include <QPainter>
#include <QString>

#include "engine/engine.hpp"
#include "ui/plotwidget.hpp"

void PlotWidget::draw(PlotData const & plotData, RedrawInfo & info)
{
    imageBuffer = QImage(plotData.imageWidth, plotData.imageHeight, QImage::Format_RGB888);
    QColor blank(64, 64, 64);
    imageBuffer.fill(blank);
    setFixedSize(plotData.imageWidth, plotData.imageHeight);
    repaint();

    redraw(plotData, info,
            [this](int x, int y, double r, double g, double b)
            {
                imageBuffer.setPixelColor(x, y, QColor(int(r*255.9), int(g*255.9), int(b*255.9)));
            });

    repaint();
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
