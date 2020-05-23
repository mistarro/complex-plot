#ifndef COMPLEXPLOT_ENGINE_HPP
#define COMPLEXPLOT_ENGINE_HPP

#include <atomic>
#include <chrono>
#include <vector>

#include "coloring.hpp"
#include "function.hpp"
#include "plotdata.hpp"

template <typename T>
class Grid
{
public:
    explicit Grid(std::size_t w, std::size_t h, T fill = T()) :
        w(w), h(h), data(w*h, fill)
    {}

    T & operator()(std::size_t x, std::size_t y) { return data[y*w + x]; }
    T const & operator()(std::size_t x, std::size_t y) const { return data[y*w + x]; }

private:
    std::size_t w;
    std::size_t h;

    std::vector<T> data;
};

template <typename UpdateFunc, typename NotifyExitFunc>
RedrawInfo redraw(PlotData const & plotData, UpdateFunc update, NotifyExitFunc notifyExit, std::atomic_bool const & cancellationToken)
{
    RedrawInfo info;

    auto start_time = std::chrono::system_clock::now();
    Function f;
    try
    {
        f.fromFormula(plotData.formula);
    }
    catch (std::invalid_argument const & e)
    {
        info.status = RedrawInfo::Status::ERROR;
        info.message = std::string("Formula error: ") + e.what() + ".";
        notifyExit();
        return info;
    }

    auto parsing_done_time = std::chrono::system_clock::now();

    Grid<complex> values(plotData.imageWidth, plotData.imageHeight, 0.0);

    for (int j = 0; j < plotData.imageHeight && !cancellationToken; ++j)
    for (int i = 0; i < plotData.imageWidth && !cancellationToken; ++i)
    {
        // compute complex argument for the pixel at (i, j)
        double re, im;
        plotData.image2complex(i, j, re, im);

        // compute value
        values(i, j) = f(complex(re, im));
    }

    auto computing_done_time = std::chrono::system_clock::now();

    for (int j = 0; j < plotData.imageHeight && !cancellationToken; ++j)
    for (int i = 0; i < plotData.imageWidth && !cancellationToken; ++i)
    {
        // compute color
        double r, g, b;
        complex2rgb_HL(values(i, j), plotData.colorSlope, r, g, b);
        update(i, j, r, g, b);
    }

    auto coloring_done_time = std::chrono::system_clock::now();

    info.parsingDuration = parsing_done_time - start_time;
    info.computingDuration = computing_done_time - parsing_done_time;
    info.coloringDuration = coloring_done_time - computing_done_time;

    info.status = cancellationToken ? RedrawInfo::Status::CANCELLED : RedrawInfo::Status::FINISHED;

    notifyExit();
    return info;
}

#endif // COMPLEXPLOT_ENGINE_HPP
