#pragma once

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <type_traits>

#include "Visitor.hpp"

namespace tree {

template <typename Visitor>
struct Node
{
    Node() = default;
    virtual ~Node() = default;
    virtual void accept(Visitor & visitor) = 0;
    virtual void traversePostorder(Visitor & visitor) = 0;

protected:
    Node(Node const &) = delete;
    Node(Node &&) = delete;
    void operator=(Node const &) = delete;
    void operator=(Node &&) = delete;
};

template <typename Visitor, std::size_t Arity>
struct NodeImpl : Node<Visitor>
{
    static constexpr std::size_t arity = Arity;

    template <typename ... Args>
    explicit NodeImpl(Args && ... args) :
        args{std::forward<Args>(args)...}
    {
        static_assert(sizeof...(Args) == Arity, "OpNode::OpNode(Args && ...): number of arguments does not match Arity.");
    }

    void traversePostorder(Visitor & visitor) override
    {
        for (auto & arg : args)
            arg->traversePostorder(visitor);
        this->accept(visitor);
    }

    template <std::size_t Index>
    std::shared_ptr<Node<Visitor>> getArg() const
    {
        static_assert(Index < Arity, "OpNode::getArg: Index < Arity does not hold.");
        return args[Index];
    }

protected:
    std::array<std::shared_ptr<Node<Visitor>> const, Arity> args;
};

template <typename Visitor>
struct NodeImpl<Visitor, 0> : Node<Visitor>
{
    static constexpr std::size_t arity = 0;
    void traversePostorder(Visitor & visitor) override { this->accept(visitor); }
};

template <typename Visitor, std::size_t Arity, typename DerivedNode>
struct VisitableNode : visitor::Visitable<Visitor, NodeImpl<Visitor, Arity>, DerivedNode>
{
    using visitor::Visitable<Visitor, NodeImpl<Visitor, Arity>, DerivedNode>::Visitable;

    template <typename ... Args>
    static std::shared_ptr<DerivedNode> create(Args && ... args)
    {
        return std::make_shared<DerivedNode>(std::forward<Args>(args)...);
    }
};

} // namespace tree

template <typename ScalarType, typename IntegerType = std::uint16_t>
struct Polynomial
{
    using Scalar = ScalarType;
    using Integer = IntegerType;

    struct Visitor;

    using Node = tree::Node<Visitor>;

    template <std::size_t Arity, typename DerivedNode>
    using VisitableNode = tree::VisitableNode<Visitor, Arity, DerivedNode>;

    struct AddNode : VisitableNode<2, AddNode> { using VisitableNode<2, AddNode>::VisitableNode; };
    struct SubNode : VisitableNode<2, SubNode> { using VisitableNode<2, SubNode>::VisitableNode; };
    struct NegNode : VisitableNode<1, NegNode> { using VisitableNode<1, NegNode>::VisitableNode; };
    struct MulNode : VisitableNode<2, MulNode> { using VisitableNode<2, MulNode>::VisitableNode; };

    template <typename DerivedNode>
    struct UniqueNode : VisitableNode<0, DerivedNode>
    {
        static std::shared_ptr<DerivedNode> create()
        {
            static std::shared_ptr<DerivedNode> const value = std::make_shared<DerivedNode>();
            return value;
        }
    };

    struct ArgNode : UniqueNode<ArgNode> {};
    struct ValNode : UniqueNode<ValNode> {};

    struct NumNode : VisitableNode<0, NumNode>
    {
        explicit NumNode(Scalar a) : value(std::move(a)) {}
        Scalar const value;

        static std::shared_ptr<NumNode> const & const0()
        {
            static std::shared_ptr<NumNode> const value = std::make_shared<NumNode>(0);
            return value;
        }

        static std::shared_ptr<NumNode> const & const1()
        {
            static std::shared_ptr<NumNode> const value = std::make_shared<NumNode>(1);
            return value;
        }

        static std::shared_ptr<NumNode> create(Scalar a)
        {
            if (a == Scalar(0))
                return const0();
            if (a == Scalar(1))
                return const1();
            return std::make_shared<NumNode>(std::move(a));
        }
    };

    struct PowNode : VisitableNode<1, PowNode>
    {
        explicit PowNode(std::shared_ptr<Node> base, Integer exp) :
            VisitableNode<1, PowNode>(std::move(base)),
            exp(std::move(exp))
        {}

        Integer const exp;
    };

    struct Visitor : visitor::VisitorBase<AddNode, SubNode, NegNode, MulNode, PowNode, ArgNode, ValNode, NumNode> {};

    Polynomial(std::shared_ptr<Node> root) : root(std::move(root)) {}

    void accept(Visitor & visitor) { root->accept(visitor); }
    void traversePostorder(Visitor & visitor) { root->traversePostorder(visitor); }

    // convenience layer

    static bool isScalar(Node & node)
    {
        struct IsScalar : Visitor
        {
            void visit(NumNode & node) override { result = true; }
            bool result;
        };

        static IsScalar isScalarVisitor;
        isScalarVisitor.result = false;
        node.accept(isScalarVisitor);
        return isScalarVisitor.result;
    }

    friend std::shared_ptr<Node> operator+(std::shared_ptr<Node> const & lhs, std::shared_ptr<Node> const & rhs)
    {
        if (lhs == NumNode::const0())
            return rhs;
        if (rhs == NumNode::const0())
            return lhs;
        if (isScalar(*lhs) && isScalar(*rhs))
        {
            Scalar a = static_cast<NumNode &>(*lhs).value;
            Scalar b = static_cast<NumNode &>(*rhs).value;
            return NumNode::create(a + b);
        }

        return AddNode::create(lhs, rhs);
    }

    friend std::shared_ptr<Node> operator+(std::shared_ptr<Node> const & lhs, Scalar a)
    {
        return lhs + NumNode::create(std::move(a));
    }

    friend std::shared_ptr<Node> operator+(Scalar a, std::shared_ptr<Node> const & rhs)
    {
        return NumNode::create(std::move(a)) + rhs;
    }

    friend std::shared_ptr<Node> operator-(std::shared_ptr<Node> const & lhs, std::shared_ptr<Node> const & rhs)
    {
        if (lhs == NumNode::const0())
            return -rhs;
        if (rhs == NumNode::const0())
            return lhs;
        if (isScalar(*lhs) && isScalar(*rhs))
        {
            Scalar a = static_cast<NumNode &>(*lhs).value;
            Scalar b = static_cast<NumNode &>(*rhs).value;
            return NumNode::create(a - b);
        }

        return SubNode::create(lhs, rhs);
    }

    friend std::shared_ptr<Node> operator-(std::shared_ptr<Node> const & lhs, Scalar a)
    {
        return lhs - NumNode::create(std::move(a));
    }

    friend std::shared_ptr<Node> operator-(Scalar a, std::shared_ptr<Node> const & rhs)
    {
        return NumNode::create(std::move(a)) - rhs;
    }

    friend std::shared_ptr<Node> operator-(std::shared_ptr<Node> const & arg)
    {
        if (isScalar(*arg))
        {
            Scalar a = static_cast<NumNode &>(*arg).value;
            return NumNode::create(-a);
        }

        return NegNode::create(arg);
    }

    friend std::shared_ptr<Node> operator*(std::shared_ptr<Node> const & lhs, std::shared_ptr<Node> const & rhs)
    {
        if (lhs == NumNode::const0() || rhs == NumNode::const1())
            return lhs;
        if (rhs == NumNode::const0() || lhs == NumNode::const1())
            return rhs;
        if (isScalar(*lhs) && isScalar(*rhs))
        {
            Scalar a = static_cast<NumNode &>(*lhs).value;
            Scalar b = static_cast<NumNode &>(*rhs).value;
            return NumNode::create(a*b);
        }

        return MulNode::create(lhs, rhs);
    }

    friend std::shared_ptr<Node> operator*(std::shared_ptr<Node> const & lhs, Scalar a)
    {
        return lhs*NumNode::create(std::move(a));
    }

    friend std::shared_ptr<Node> operator*(Scalar a, std::shared_ptr<Node> const & rhs)
    {
        return NumNode::create(std::move(a))*rhs;
    }

    friend std::shared_ptr<Node> pow(std::shared_ptr<Node> const & base, Integer exp)
    {
        if (exp == 0)
            return NumNode::const1();
        if (exp == 1)
            return base;
        if (isScalar(*base))
        {
            Scalar a = static_cast<NumNode &>(*base).value;
            Scalar res = 1;
            for (Integer i = 0; i < exp; ++i)
                res *= a;
            return NumNode::create(res);
        }

        return PowNode::create(base, std::move(exp));
    }

protected:
    std::shared_ptr<Node> const root;
};

template <typename Poly>
struct Printer : Poly::Visitor
{
    Printer(std::ostream & out, std::string indent = "") : out(out), indent(std::move(indent)) {}

    void visit(typename Poly::AddNode & node) override { dump_node(node, "+"); }
    void visit(typename Poly::SubNode & node) override { dump_node(node, "-"); }
    void visit(typename Poly::NegNode & node) override { dump_node(node, "-"); }
    void visit(typename Poly::MulNode & node) override { dump_node(node, "*"); }
    void visit(typename Poly::PowNode & node) override { dump_node(node, "^" + to_string(node.exp)); }
    void visit(typename Poly::NumNode & node) override { dump_node(node, to_string(node.value)); }
    void visit(typename Poly::ArgNode & node) override { dump_node(node, "x"); }
    void visit(typename Poly::ValNode & node) override { dump_node(node, "y"); }

private:
    std::ostream & out;
    std::string indent;

    template <typename T>
    static std::string to_string(T const & t)
    {
        std::ostringstream s;
        s << t;
        return s.str();
    }

    void dump_branching(std::string const & label) { out << "[" << label << "]\n"; }

    template <bool IsLastChild>
    static std::string const & getIndentSuffix()
    {
        static const std::string value(IsLastChild ? "   " : " | ");
        return value;
    }

    template <std::size_t Index, std::size_t Arity>
    void dump_child(tree::NodeImpl<typename Poly::Visitor, Arity> & node)
    {
        out << indent << " |\n";
        out << indent << " +-";
        indent += getIndentSuffix<Index + 1 == Arity>();
        node.template getArg<Index>()->accept(*this); // recursion
        indent.resize(indent.length() - 3);
    }

    template <std::size_t Arity>
    void dump_node(tree::NodeImpl<typename Poly::Visitor, Arity> & node, std::string const & s)
    {
        dump_branching(s);
        dump_node_impl<Arity>(node, std::make_index_sequence<Arity>());
    }

    template <std::size_t Arity, std::size_t ... Seq>
    void dump_node_impl(tree::NodeImpl<typename Poly::Visitor, Arity> & node, std::index_sequence<Seq...>)
    {
        (dump_child<Seq>(node), ...);
    }
};

template <typename Poly>
void dump_tree(Poly & e, std::ostream & out, std::string indent = "")
{
    Printer<Poly> printer(out, std::move(indent));
    e.accept(printer);
}
