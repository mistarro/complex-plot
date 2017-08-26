#ifndef COMPLEXPLOT_QT_PLOTDATA_H
#define COMPLEXPLOT_QT_PLOTDATA_H

#include <string>

struct PlotData
{
  std::string formula;

  double reMin;
  double reMax;
  double imMin;
  double imMax;

  int imageWidth;
  int imageHeight;

  int coloringMethod;
  double colorSlope;
};

#endif // COMPLEXPLOT_QT_PLOTDATA_H
