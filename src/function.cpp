#include "function.h"

Function::Function(std::string const & formula)
{
  (void)formula;
}

complex Function::operator()(complex const & z) const
{
  // stub
  return z*z + 1.0;
}
