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

    Grid<std::optional<complex>> values(plotData.imageWidth, plotData.imageHeight);

    struct QEntry
    {
        int x;
        int y;
    };

    std::queue<QEntry> q;

    int x, y;
    plotData.complex2image(plotData.reSeed, plotData.imSeed, x, y);
    q.push({x, y});
    values(x, y) = complex(0.0, 0.0);

    while (!q.empty() && !cancellationToken)
    {
        QEntry & e = q.front();
        double re, im;
        plotData.image2complex(e.x, e.y, re, im);
        values(e.x, e.y) = f(complex(re, im));

        // enqueue neighbors
        if (e.x < plotData.imageWidth - 1 && !values(e.x + 1, e.y))
        {
            q.push({e.x + 1, e.y});
            values(e.x + 1, e.y) = complex(0.0, 0.0);
        }

        if (e.x > 0 && !values(e.x - 1, e.y))
        {
            q.push({e.x - 1, e.y});
            values(e.x - 1, e.y) = complex(0.0, 0.0);
        }

        if (e.y < plotData.imageHeight - 1 && !values(e.x, e.y + 1))
        {
            q.push({e.x, e.y + 1});
            values(e.x, e.y + 1) = complex(0.0, 0.0);
        }

        if (e.y > 0 && !values(e.x, e.y - 1))
        {
            q.push({e.x, e.y - 1});
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
