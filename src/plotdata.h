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

    void image2complex(int x, int y, double & re, double & im) const;
    void complex2image(double re, double im, int & x, int & y) const;
};

inline void PlotData::image2complex(int x, int y, double & re, double & im) const
{
    re = (reMin*(imageWidth - x - 0.5) + reMax*(x + 0.5))/imageWidth;
    im = (imMin*(y + 0.5) + imMax*(imageHeight - y - 0.5))/imageHeight;
}

inline void PlotData::complex2image(double re, double im, int & x, int & y) const
{
    int xx = (re - reMin)/(reMax - reMin)*imageWidth;
    int yy = (imMax - im)/(imMax - imMin)*imageHeight;
    x = std::min(std::max(xx, 0), imageWidth - 1);
    y = std::min(std::max(yy, 0), imageHeight - 1);
}

#endif // COMPLEXPLOT_QT_PLOTDATA_H
