#include <QPainter>
#include <QColor>

#include "plotwidget.h"
#include "coloring.h"
#include "function.h"

void PlotWidget::redraw(PlotData const & plotData)
{
    imageBuffer = QImage(plotData.imageWidth, plotData.imageHeight, QImage::Format_RGB888);
    Function f(plotData.formula.toStdString());

    for (int j = 0; j < imageBuffer.height(); ++j)
    for (int i = 0; i < imageBuffer.width(); ++i)
    {
        // compute complex argument for the pixel at (i, j)
        std::complex<double> z(
            (plotData.reMax - plotData.reMin)*i/plotData.imageWidth + plotData.reMin,
            (plotData.imMin - plotData.imMax)*j/plotData.imageHeight + plotData.imMax
        );

        // compute value
        complex value = f(z);

        // compute color
        double r, g, b;
        complex2rgb_HL(value, plotData.colorSlope, r, g, b);

        imageBuffer.setPixelColor(i, j, QColor(int(r*255.9), int(g*255.9), int(b*255.9)));
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
