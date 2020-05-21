#ifndef COMPLEXPLOT_ENGINE_HPP
#define COMPLEXPLOT_ENGINE_HPP

#include <chrono>
#include <vector>

#include "coloring.hpp"
#include "function.hpp"
#include "plotdata.hpp"

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

#endif // COMPLEXPLOT_ENGINE_HPP
