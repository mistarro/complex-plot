#include <QPainter>
#include <QColor>

#include "plotwidget.h"

void PlotWidget::redraw(PlotData const & plotData)
{
    imageBuffer = QImage(plotData.imageWidth, plotData.imageHeight, QImage::Format_RGB888);

    for (int j = 0; j < imageBuffer.height(); ++j)
    for (int i = 0; i < imageBuffer.width(); ++i)
    {
          imageBuffer.setPixelColor(i, j, QColor(i&0xff, j&0xff, (i+j)&0xff));
    }

    this->setFixedSize(plotData.imageWidth, plotData.imageHeight);
    this->update();
}

void PlotWidget::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.drawImage(0, 0, imageBuffer);
}
