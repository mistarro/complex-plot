#ifndef COMPLEXPLOT_ENGINE_HPP
#define COMPLEXPLOT_ENGINE_HPP

#include <atomic>
#include <chrono>
#include <optional>
#include <queue>
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

inline void checkPlotData(PlotData const & plotData, Function & f, RedrawInfo & info)
{
    if (plotData.reMin >= plotData.reMax ||
        plotData.imMin >= plotData.imMax)
    {
        info.status = RedrawInfo::Status::ERROR;
        info.message = "Complex plane range error.";
        return;
    }

    if (plotData.reSeed < plotData.reMin ||
        plotData.reSeed > plotData.reMax ||
        plotData.imSeed < plotData.imMin ||
        plotData.imSeed > plotData.imMax)
    {
        info.status = RedrawInfo::Status::ERROR;
        info.message = "Seed argument not in range.";
        return;
    }

    if (!f.fromFormula(plotData.formula))
    {
        info.status = RedrawInfo::Status::ERROR;
        info.message = f.getErrorMessage();
        return;
    }
}

template <typename UpdateFunc, typename NotifyExitFunc>
RedrawInfo redraw(PlotData const & plotData, UpdateFunc update, NotifyExitFunc notifyExit, std::atomic_bool const & cancellationToken)
{
    RedrawInfo info;

    auto start_time = std::chrono::system_clock::now();

    Function f;

    // check plot data
    info.status = RedrawInfo::Status::CANCELLED;
    checkPlotData(plotData, f, info);
    if (info.status == RedrawInfo::Status::ERROR)
    {
        notifyExit();
        return info;
    }

    auto parsing_done_time = std::chrono::system_clock::now();

    Grid<std::optional<complex>> values(plotData.imageWidth, plotData.imageHeight);

    struct QEntry
    {
        int x;
        int y;
        complex w0;
    };

    std::queue<QEntry> q;

    int x, y;
    plotData.complex2image(plotData.reSeed, plotData.imSeed, x, y);
    q.push({x, y, complex(plotData.reSeedValue, plotData.imSeedValue)});
    values(x, y) = complex(0.0, 0.0);

    while (!q.empty() && !cancellationToken)
    {
        QEntry & e = q.front();
        double re, im;
        plotData.image2complex(e.x, e.y, re, im);
        complex w = f(complex(re, im), e.w0);
        values(e.x, e.y) = w;

        // enqueue neighbors
        if (e.x < plotData.imageWidth - 1 && !values(e.x + 1, e.y))
        {
            q.push({e.x + 1, e.y, w});
            values(e.x + 1, e.y) = complex(0.0, 0.0);
        }

        if (e.x > 0 && !values(e.x - 1, e.y))
        {
            q.push({e.x - 1, e.y, w});
            values(e.x - 1, e.y) = complex(0.0, 0.0);
        }

        if (e.y < plotData.imageHeight - 1 && !values(e.x, e.y + 1))
        {
            q.push({e.x, e.y + 1, w});
            values(e.x, e.y + 1) = complex(0.0, 0.0);
        }

        if (e.y > 0 && !values(e.x, e.y - 1))
        {
            q.push({e.x, e.y - 1, w});
            values(e.x, e.y - 1) = complex(0.0, 0.0);
        }

        q.pop();
    }

    auto computing_done_time = std::chrono::system_clock::now();

    for (int j = 0; j < plotData.imageHeight && !cancellationToken; ++j)
    for (int i = 0; i < plotData.imageWidth && !cancellationToken; ++i)
    {
        // compute color
        double r, g, b;
        complex2rgb_HL(*values(i, j), plotData.colorSlope, r, g, b);
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
