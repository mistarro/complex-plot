#include <cassert>
#include <complex>
#include <cmath>

namespace {

// 3/pi constant
double const M_3_PI = 0.954929658551372015;

// Given complex number z returns color hue in [-3.0, 3.0]
inline double hue(std::complex<double> z)
{
    return M_3_PI*std::arg(z);
}

// hue-p-q to component intensity helper function
// h - positive hue
inline double hpq2c(double h, double p, double q)
{
    h = std::fmod(h, 6.0);

    if (h < 1.0)
        return p + (q - p)*h;
    if (h < 3.0)
        return q;
    if (h < 4.0)
        return p + (q - p)*(4.0 - h);
    return p;
}

// Classic hue/lightness to rgb
//   h: hue value in [-3.0, 3.0]
//   l: lightness in [0.0, 2.0]
//   r, g, b: out parameters in [0.0, 1.0]
inline void hl2rgb(double h, double l, double & r, double & g, double & b)
{
    assert(h >= -3.0);
    assert(h <= 3.0);
    assert(l >= 0.0);
    assert(l <= 2.0);

    double q = std::fmin(l, 1.0);
    double p = l - q;

    r = hpq2c(h +  8.0, p, q);
    g = hpq2c(h +  6.0, p, q);
    b = hpq2c(h + 10.0, p, q);

    assert(r >= 0.0);
    assert(r <= 1.0);
    assert(g >= 0.0);
    assert(g <= 1.0);
    assert(b >= 0.0);
    assert(b <= 1.0);
}

inline double lightness_HL(std::complex<double> z, double a)
{
    return 2.0/(std::pow(std::abs(z), a) + 1.0);
}

} // namespace

void complex2rgb_HL(std::complex<double> z, double a, double & r, double & g, double & b)
{
    if (std::isnan(z.real()) || std::isnan(z.real()))
    {
        r = g = b = 0.5;
        return;
    }

    hl2rgb(hue(z), lightness_HL(z, a), r, g, b);
}
