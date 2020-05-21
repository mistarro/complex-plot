#ifndef COMPLEXPLOT_COLORING_HPP
#define COMPLEXPLOT_COLORING_HPP

#include <complex>

/*
 *  void complex2rgb_*(std::complex<double> z, double a, double & r, double & g, double & b)
 *  arguments:
 *    z - complex number
 *    a - lightness slope (roughly how fast lightness tends to white/black as a point goes to 0/infinity);
 *    r, g, b - components of the computed color in [0.0, 1.0];
 */

void complex2rgb_HL(std::complex<double>, double, double &, double &, double &);

#endif // COMPLEXPLOT_COLORING_HPP
