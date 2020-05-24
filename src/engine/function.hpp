#ifndef COMPLEXPLOT_FUNCTION_HPP
#define COMPLEXPLOT_FUNCTION_HPP

#include <complex>
#include <functional>
#include <string>

// JIT
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>

using complex = std::complex<double>;

class Function
{
public:
    Function(std::string const & formula);

    complex operator()(complex z, complex w) const;

    operator bool() const { return f != nullptr; }
    std::string const & getErrorMessage() const { return message; }

private:
    // TODO: make configurable
    static constexpr std::uint16_t maxIterations = 100;
    static constexpr double epsilon = 1.0e-4;

    std::unique_ptr<llvm::orc::LLJIT> lljit;
    llvm::ExitOnError exitOnErr;

    using PolyFunPtrType = void (*)(double, double, double, double, double &, double &, double &, double &, double &, double &);

    PolyFunPtrType f;

    void fromFormula(std::string const & formula);

    static double & re(complex & z)
    {
        return reinterpret_cast<double(&)[2]>(z)[0];
    }

    static double & im(complex & z)
    {
        return reinterpret_cast<double(&)[2]>(z)[1];
    }

    void fnHelper(complex const & z, complex const & w, complex & v0, complex & v1, complex & v2) const
    {
        f(z.real(), z.imag(), w.real(), w.imag(), re(v0), im(v0), re(v1), im(v1), re(v2), im(v2));
    }

    std::string message;
};

#endif // COMPLEXPLOT_FUNCTION_HPP
