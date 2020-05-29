#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

// IR gen
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
// optimize
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include "function.hpp"

#include "Parser.hpp"
#include "Polynomial.hpp"

Function::Function(std::string const & formula) :
    f(nullptr),
    message("Success")
{
    // initialize LLVM and LLJIT
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    lljit = exitOnErr(llvm::orc::LLJITBuilder().create());

    try
    {
        fromFormula(formula);
    }
    catch (std::invalid_argument const & e)
    {
        message = std::string("Formula error: ") + e.what() + ".";
    }
}

// compute square root "in direction" of b
inline complex sqrt(complex z, complex b)
{
    double h = std::abs(z);
    double x = std::sqrt((h + z.real())/2);
    double y = std::sqrt((h - z.real())/2);
    y = std::copysign(y, z.imag());
    double s = std::copysign(1.0, x*b.real() + y*b.imag());
    return complex(x*s, y*s);
}

inline double norm_1(complex z)
{
    return std::abs(z.real()) + std::abs(z.imag());
}

complex Function::operator()(complex z, complex w) const
{
    for (std::uint16_t i = 0; i < maxIterations; ++i)
    {
        complex d0, d1, d2;
        fnHelper(z, w, d0, d1, d2);
        if (norm_1(d0) < std::numeric_limits<double>::epsilon())
            return w;
        // Laguerre's method
        complex G = d1/d0;
        complex G2 = G*G;
        complex H = G2 - d2/d0;
        complex tmp = degree*H - G2;
        tmp = sqrt(tmp*(degree - 1.0), G);
        complex correction = degree/(G + tmp);
        w -= correction;
        // check negation to also break when correction is NaN
        if (!(norm_1(correction) > epsilon*(1.0 + norm_1(w))))
            return w;
    }
}

namespace {

using Float = complex::value_type;
using Int = std::uint16_t;

using Poly = Polynomial<complex, Int>;

struct ParserTraits
{
    using FloatType = Float;
    using IntegerType = Int;
    using ResultType = std::shared_ptr<Poly::Node>;

    ResultType onAdd(ResultType lhs, ResultType rhs) { return lhs + rhs; }
    ResultType onSub(ResultType lhs, ResultType rhs) { return lhs - rhs; }
    ResultType onNeg(ResultType arg) { return -arg; }
    ResultType onMul(ResultType lhs, ResultType rhs) { return lhs*rhs; }
    ResultType onPow(ResultType base, IntegerType exp) { return pow(base, exp); }
    ResultType onFloat(FloatType a) { return Poly::NumNode::create(complex(a, 0.0)); }
    ResultType onI() { return Poly::NumNode::create(complex(0.0, 1.0)); }
    ResultType onZ() { return Poly::ArgNode::create(); }
    ResultType onW() { return Poly::ValNode::create(); }
};

struct GenLLVMIR : Poly::Visitor
{
    GenLLVMIR(llvm::IRBuilder<> & builder, llvm::Function * function) :
        builder(builder),
        function(function),
        f0(llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(0.0))),
        f1(llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(1.0)))
    {}

    struct LLVMComplexValue
    {
        llvm::Value * re;
        llvm::Value * im;
    };

    LLVMComplexValue createCAdd(LLVMComplexValue lhs, LLVMComplexValue rhs)
    {
        LLVMComplexValue res;
        res.re = builder.CreateFAdd(lhs.re, rhs.re);
        res.im = builder.CreateFAdd(lhs.im, rhs.im);
        return res;
    }

    LLVMComplexValue createCSub(LLVMComplexValue lhs, LLVMComplexValue rhs)
    {
        LLVMComplexValue res;
        res.re = builder.CreateFSub(lhs.re, rhs.re);
        res.im = builder.CreateFSub(lhs.im, rhs.im);
        return res;
    }

    LLVMComplexValue createCNeg(LLVMComplexValue arg)
    {
        LLVMComplexValue res;
        res.re = builder.CreateFNeg(arg.re);
        res.im = builder.CreateFNeg(arg.im);
        return res;
    }

    LLVMComplexValue createCMul(llvm::Value * lhs, LLVMComplexValue rhs)
    {
        LLVMComplexValue res;
        res.re = builder.CreateFMul(lhs, rhs.re);
        res.im = builder.CreateFMul(lhs, rhs.im);
        return res;
    }

    LLVMComplexValue createCMul(LLVMComplexValue lhs, LLVMComplexValue rhs)
    {
        LLVMComplexValue res;
        llvm::Value * rr = builder.CreateFMul(lhs.re, rhs.re);
        llvm::Value * ii = builder.CreateFMul(lhs.im, rhs.im);
        llvm::Value * ri = builder.CreateFMul(lhs.re, rhs.im);
        llvm::Value * ir = builder.CreateFMul(lhs.im, rhs.re);
        res.re = builder.CreateFSub(rr, ii);
        res.im = builder.CreateFAdd(ri, ir);
        return res;
    }

    LLVMComplexValue createCPow(LLVMComplexValue base, Poly::Integer exp)
    {
        if (exp == 0)
            return {f1, f0};
        auto result = createCPow(base, exp/2);
        result = createCMul(result, result);
        if (exp%2 != 0)
            result = createCMul(result, base);
        return result;
    }

    LLVMComplexValue createCConst(complex c)
    {
        LLVMComplexValue res;
        res.re = llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(c.real()));
        res.im = llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(c.imag()));
        return res;
    }

    void visit(Poly::NumNode & node) override
    {
        res[0] = createCConst(node.value);
        res[1] = {f0, f0};
        res[2] = {f0, f0};
    }

    void visit(Poly::ArgNode & node) override
    {
        res[0].re = function->getArg(0);
        res[0].im = function->getArg(1);
        res[1] = {f0, f0};
        res[2] = {f0, f0};
    }

    void visit(Poly::ValNode & node) override
    {
        res[0].re = function->getArg(2);
        res[0].im = function->getArg(3);
        res[1] = {f1, f0};
        res[2] = {f0, f0};
    }

    void visit(Poly::AddNode & node) override
    {
        node.getArg<0>()->accept(*this);
        auto lhs = res[0];
        auto lhsDer = res[1];
        auto lhsDer2 = res[2];
        node.getArg<1>()->accept(*this);
        auto rhs = res[0];
        auto rhsDer = res[1];
        auto rhsDer2 = res[2];

        res[0] = createCAdd(lhs, rhs);
        res[1] = createCAdd(lhsDer, rhsDer);
        res[2] = createCAdd(lhsDer2, rhsDer2);
    }

    void visit(Poly::SubNode & node) override
    {
        node.getArg<0>()->accept(*this);
        auto lhs = res[0];
        auto lhsDer = res[1];
        auto lhsDer2 = res[2];
        node.getArg<1>()->accept(*this);
        auto rhs = res[0];
        auto rhsDer = res[1];
        auto rhsDer2 = res[2];

        res[0] = createCSub(lhs, rhs);
        res[1] = createCSub(lhsDer, rhsDer);
        res[2] = createCSub(lhsDer2, rhsDer2);
    }

    void visit(Poly::NegNode & node) override
    {
        node.getArg<0>()->accept(*this);
        res[0] = createCNeg(res[0]);
        res[1] = createCNeg(res[1]);
        res[2] = createCNeg(res[2]);
    }

    void visit(Poly::MulNode & node) override
    {
        node.getArg<0>()->accept(*this);
        auto lhs = res[0];
        auto lhsDer = res[1];
        auto lhsDer2 = res[2];
        node.getArg<1>()->accept(*this);
        auto rhs = res[0];
        auto rhsDer = res[1];
        auto rhsDer2 = res[2];

        res[0] = createCMul(lhs, rhs);
        auto lhs_rhsDer = createCMul(lhs, rhsDer);
        auto lhsDer_rhs = createCMul(lhsDer, rhs);
        res[1] = createCAdd(lhs_rhsDer, lhsDer_rhs);
        auto lhsDer2_rhs = createCMul(lhsDer2, rhs);
        auto lhsDer_rhsDer = createCMul(lhsDer, rhsDer);
        auto lhs_rhsDer2 = createCMul(lhs, rhsDer2);
        res[2] = createCAdd(lhsDer2_rhs, lhs_rhsDer2);
        res[2] = createCAdd(res[2], lhsDer_rhsDer);
        res[2] = createCAdd(res[2], lhsDer_rhsDer);
    }

    void visit(Poly::PowNode & node) override
    {
        if (node.exp == 0)
        {
            res[0] = {f1, f0};
            res[1] = {f0, f0};
            res[2] = {f0, f0};
            return;
        }

        node.getArg<0>()->accept(*this);
        if (node.exp == 1)
            return;

        auto base = res[0];
        auto baseDer = res[1];
        auto exp = llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(double(node.exp)));
        auto expm1 = llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(double(node.exp - 1)));
        auto base_em2 = createCPow(base, node.exp - 2);
        auto base_em1 = createCMul(base, base_em2);
        res[0] = createCMul(base_em1, base);
        auto e_base_em2 = createCMul(exp, base_em2);
        res[1] = createCMul(e_base_em2, base);
        res[1] = createCMul(res[1], baseDer);
        auto baseDer2 = createCMul(baseDer, baseDer);
        auto em1_baseDer2 = createCMul(expm1, baseDer2);
        auto base_baseDerDer = createCMul(base, res[2]);
        res[2] = createCAdd(em1_baseDer2, base_baseDerDer);
        res[2] = createCMul(e_base_em2, res[2]);
    }

    llvm::Value * genPow(llvm::Value * base, Poly::Integer exp)
    {
        if (exp == 0)
            return llvm::ConstantFP::get(builder.getContext(), llvm::APFloat(1.0));
        auto result = genPow(base, exp/2);
        result = builder.CreateFMul(result, result);
        if (exp%2 != 0)
            result = builder.CreateFMul(result, base);
        return result;
    }

    llvm::IRBuilder<> & builder;
    llvm::Function * function;

    llvm::Constant * const f0;
    llvm::Constant * const f1;

    LLVMComplexValue res[3];
};

struct PolyDegVal : Poly::Visitor
{
    PolyDegVal() :
        deg(0)
    {}

    void visit(Poly::NumNode & node) override { deg = 0; }
    void visit(Poly::ArgNode & node) override { deg = 0; }
    void visit(Poly::ValNode & node) override { deg = 1; }
    void visit(Poly::AddNode & node) override
    {
        node.getArg<0>()->accept(*this);
        auto lhs = deg;
        node.getArg<1>()->accept(*this);
        auto rhs = deg;
        deg = std::max(lhs, rhs);
    }

    void visit(Poly::SubNode & node) override
    {
        node.getArg<0>()->accept(*this);
        auto lhs = deg;
        node.getArg<1>()->accept(*this);
        auto rhs = deg;
        deg = std::max(lhs, rhs);
    }

    void visit(Poly::NegNode & node) override {}

    void visit(Poly::MulNode & node) override
    {
        node.getArg<0>()->accept(*this);
        auto lhs = deg;
        node.getArg<1>()->accept(*this);
        auto rhs = deg;
        deg = lhs + rhs;
    }

    void visit(Poly::PowNode & node) override
    {
        node.getArg<0>()->accept(*this);
        deg *= node.exp;
    }

    int deg;
};

} // namespace

void Function::fromFormula(std::string const & formula)
{
    Parser<ParserTraits> parser(formula.cbegin(), formula.cend());
    Poly poly(parser.parse());

    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("complex-algebraic-plot-LLVM-IR", *ctx);

    llvm::IRBuilder<> builder(*ctx);

    // allow more aggresive optimizations
    llvm::FastMathFlags fmf;
    fmf.setAllowReassoc();
    fmf.setNoSignedZeros();
    fmf.setNoNaNs();
    fmf.setNoInfs();
    fmf.setAllowContract();
    builder.setFastMathFlags(fmf);

    // signature
    llvm::Type * doubleType = llvm::Type::getDoubleTy(*ctx);
    llvm::Type * doublePtrType = llvm::PointerType::getUnqual(doubleType);
    std::vector<llvm::Type *> argTypes{
        doubleType,
        doubleType,
        doubleType,
        doubleType,
        doublePtrType,
        doublePtrType,
        doublePtrType,
        doublePtrType,
        doublePtrType,
        doublePtrType
    };
    llvm::Type * voidType = llvm::Type::getVoidTy(*ctx);
    llvm::FunctionType * fType = llvm::FunctionType::get(voidType, argTypes, false);

    llvm::Function * function = llvm::Function::Create(fType, llvm::Function::ExternalLinkage, "fun", *module);
    function->getArg(0)->setName("zRe");
    function->getArg(1)->setName("zIm");
    function->getArg(2)->setName("wRe");
    function->getArg(3)->setName("wIm");
    function->getArg(4)->setName("d0Re");
    function->getArg(5)->setName("d0Im");
    function->getArg(6)->setName("d1Re");
    function->getArg(7)->setName("d1Im");
    function->getArg(8)->setName("d2Re");
    function->getArg(9)->setName("d2Im");
    llvm::BasicBlock * entry = llvm::BasicBlock::Create(*ctx, "entry", function);
    builder.SetInsertPoint(entry);

    // generate body
    GenLLVMIR gen(builder, function);
    poly.accept(gen);

    builder.CreateStore(gen.res[0].re, function->getArg(4));
    builder.CreateStore(gen.res[0].im, function->getArg(5));
    builder.CreateStore(gen.res[1].re, function->getArg(6));
    builder.CreateStore(gen.res[1].im, function->getArg(7));
    builder.CreateStore(gen.res[2].re, function->getArg(8));
    builder.CreateStore(gen.res[2].im, function->getArg(9));
    builder.CreateRetVoid();
    llvm::verifyFunction(*function);

    // optimize
    // TODO: switch to new llvm::PassManager<llvm::Function>
    llvm::legacy::FunctionPassManager fpm(module.get());

    fpm.add(llvm::createInstructionCombiningPass());
    fpm.add(llvm::createReassociatePass());
    fpm.add(llvm::createNaryReassociatePass());
    fpm.add(llvm::createConstantPropagationPass());
    fpm.add(llvm::createGVNPass()); // Eliminate Common SubExpressions.
    fpm.add(llvm::createCFGSimplificationPass()); // Simplify the control flow graph (deleting unreachable blocks, etc).
    fpm.doInitialization();
    fpm.run(*function);

    // print optimized
    std::cout << ">>>> Optimized code:" << std::endl;
    module->print(llvm::outs(), nullptr);

    exitOnErr(lljit->addIRModule(llvm::orc::ThreadSafeModule(std::move(module), std::move(ctx))));
    auto polySym = exitOnErr(lljit->lookup("fun"));
    f = reinterpret_cast<PolyFunPtrType>(polySym.getAddress());

    // compute w-degree
    PolyDegVal degVisitor;
    poly.accept(degVisitor);
    degree = degVisitor.deg;
    std::cout << ">>>> w-degree: " << degVisitor.deg << std::endl;
}
