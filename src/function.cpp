#include "function.h"

void Function::fromFormula(std::string const & formula)
{
  (void)formula;
}

complex Function::operator()(complex const & z) const
{
  // stub
  return z*z + 1.0;
}
