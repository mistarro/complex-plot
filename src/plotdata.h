#ifndef COMPLEXPLOT_QT_PLOTDATA_H
#define COMPLEXPLOT_QT_PLOTDATA_H

struct PlotData
{
    QString formula;

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