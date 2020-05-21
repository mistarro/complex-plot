#ifndef COMPLEXPLOT_FUNCTION_HPP
#define COMPLEXPLOT_FUNCTION_HPP

#include <complex>
#include <deque>
#include <functional>
#include <iostream>
#include <map>
#include <string>

using complex = std::complex<double>;

class Expression
{
public:
    Expression() : root(nullptr) {}

    using NodeFunction = std::function<complex(complex const &, complex const &)>;

    struct Node
    {
        Node * left;
        Node * right;

        NodeFunction fun;

        template <typename F>
        Node(Node * left, Node * right, F && fun) :
            left(left), right(right), fun(fun)
        {}
    };

    void set_root(Node * root) { this->root = root; }

    complex eval(complex const & z) const { return eval(root, z); }

    template <typename ... Args>
    Node * new_Node(Args && ... args)
    {
        memory.emplace_back(std::forward<Args>(args)...);
        return &memory.back();
    }

    static std::map<std::string, NodeFunction> fun;

private:
    static complex eval(Node * const node, complex const & z)
    {
        return (node == nullptr) ? z : std::proj(node->fun(eval(node->left, z), eval(node->right, z)));
    }

    std::deque<Node> memory;
    Node * root;
};


class Function
{
public:
    void fromFormula(std::string const & formula);

    complex operator()(complex const & z) const { return expression.eval(z); }

private:
    Expression expression;
};

#endif // COMPLEXPLOT_FUNCTION_HPP
