#include <chrono>
#include <string>

#include <QColor>
#include <QPainter>
#include <QString>

#include "coloring.h"
#include "function.h"
#include "plotwidget.h"

template <typename UpdateFunc>
void redraw(PlotData const & plotData, RedrawInfo & info, UpdateFunc update)
{
    auto start_time = std::chrono::system_clock::now();
    Function f;
    f.fromFormula(plotData.formula);

    auto parsing_done_time = std::chrono::system_clock::now();

    std::vector<complex> values(plotData.imageWidth*plotData.imageHeight, 0.0);

    for (int j = 0; j < plotData.imageHeight; ++j)
    for (int i = 0; i < plotData.imageWidth; ++i)
    {
        // compute complex argument for the pixel at (i, j)
        double re, im;
        plotData.image2complex(i, j, re, im);
        complex z(re, im);

        // compute value
        values[j*plotData.imageWidth + i] = f(z);
    }

    auto computing_done_time = std::chrono::system_clock::now();

    for (int j = 0; j < plotData.imageHeight; ++j)
    for (int i = 0; i < plotData.imageWidth; ++i)
    {
        // compute color
        double r, g, b;
        complex2rgb_HL(values[j*plotData.imageWidth + i], plotData.colorSlope, r, g, b);
        update(i, j, r, g, b);
    }

    auto coloring_done_time = std::chrono::system_clock::now();

    info.parsingDuration = parsing_done_time - start_time;
    info.computingDuration = computing_done_time - parsing_done_time;
    info.coloringDuration = coloring_done_time - computing_done_time;
}

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
