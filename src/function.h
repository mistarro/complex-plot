#ifndef COMPLEXPLOT_FUNCTION_H
#define COMPLEXPLOT_FUNCTION_H

#include <complex>
#include <string>

using complex = std::complex<double>;

class Function
{
public:
  Function(std::string const & formula);

  complex operator()(complex const &) const;
};


#endif // COMPLEXPLOT_FUNCTION_H
