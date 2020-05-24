#pragma once

namespace visitor {

template <typename... Types>
struct VisitorBase;

template <typename T>
struct VisitorBase<T>
{
    virtual ~VisitorBase() = default;
    virtual void visit(T &) {}
};

template <typename T, typename... Types>
struct VisitorBase<T, Types...> : VisitorBase<Types...>
{
    using VisitorBase<Types...>::visit;
    virtual void visit(T &) {}
};

template <typename Visitor, typename Base, typename Derived>
struct Visitable : Base
{
    using Base::Base;
    void accept(Visitor & visitor) override { visitor.visit(static_cast<Derived &>(*this)); }
};

} // namespace visitor
