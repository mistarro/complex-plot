#include <string>
#include <chrono>

#include <QString>
#include <QPainter>
#include <QColor>

#include "plotwidget.h"
#include "coloring.h"
#include "function.h"

void PlotWidget::redraw(PlotData const & plotData, RedrawInfo & info)
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
    complex z(
        (plotData.reMax - plotData.reMin)*i/plotData.imageWidth + plotData.reMin,
        (plotData.imMin - plotData.imMax)*j/plotData.imageHeight + plotData.imMax
    );

    // compute value
    values[j*plotData.imageWidth + i] = f(z);
  }

  auto computing_done_time = std::chrono::system_clock::now();

  imageBuffer = QImage(plotData.imageWidth, plotData.imageHeight, QImage::Format_RGB888);

  for (int j = 0; j < plotData.imageHeight; ++j)
  for (int i = 0; i < plotData.imageWidth; ++i)
  {
    // compute color
    double r, g, b;
    complex2rgb_HL(values[j*plotData.imageWidth + i], plotData.colorSlope, r, g, b);

    imageBuffer.setPixelColor(i, j, QColor(int(r*255.9), int(g*255.9), int(b*255.9)));
  }

  auto coloring_done_time = std::chrono::system_clock::now();

  this->setFixedSize(plotData.imageWidth, plotData.imageHeight);
  this->update();

  info.parsingDuration = parsing_done_time - start_time;
  info.computingDuration = computing_done_time - parsing_done_time;
  info.coloringDuration = coloring_done_time - computing_done_time;
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
