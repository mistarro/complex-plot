#include <cctype>
#include <vector>
#include <string>
#include <stdexcept>

#include "function.h"

// functions
std::map<std::string, Expression::NodeFunction> Expression::fun
{
  {"exp", [](complex const & a, complex const &) { return std::exp(a); }}
};

namespace {

class Lexer
{
public:
  struct Token;
  using TokenCollection = std::vector<Token>;
  using TokenIterator = TokenCollection::const_iterator;

  explicit Lexer(std::string const & input) : head(input.begin()), eos(input.end()) {}

  void tokenize();

  TokenIterator begin() const { return tokens.cbegin(); }
  TokenIterator end() const { return tokens.cend(); }

private:
  std::string::const_iterator head;
  std::string::const_iterator eos;
  std::vector<Token> tokens;
};

struct Lexer::Token
{
  enum class Type;

  Type type;
  std::string value;

  Token(Type type, std::string value = "") :
    type(type),
    value(value)
  {}

  Token(Type type, std::string::const_iterator begin, std::string::const_iterator end) :
    type(type),
    value(begin, end)
  {}
};

enum class Lexer::Token::Type
{
  ADD, MUL, POW,
  LP, RP,
  ID,
  Z,
  I,
  REAL,
  EOD,
  NONE
};

void Lexer::tokenize()
{
  while(head != eos)
  {
    // skip whitespaces
    while (std::isspace(*head)) ++head;

    auto begin = head;
    Token::Type type = Token::Type::NONE;

    // one-character
    switch(*head)
    {
    case '+':
    case '-':
      type = Token::Type::ADD;
      break;
    case '*':
    case '/':
      type = Token::Type::MUL;
      break;
    case '^':
      type = Token::Type::POW;
      break;
    case '(':
      type = Token::Type::LP;
      break;
    case ')':
      type = Token::Type::RP;
      break;
    }

    if (type != Token::Type::NONE)
    {
      ++head;
      tokens.emplace_back(type, begin, head);
      continue;
    }

    // REAL
    if (std::isdigit(*head))
    {
      ++head;
      while(std::isdigit(*head)) ++head;
      if (*head == '.')
      {
        ++head;
        while(std::isdigit(*head)) ++head;
      }

      tokens.emplace_back(Token::Type::REAL, begin, head);
      continue;
    }

    // ID, Z, I
    if (std::isalpha(*head))
    {
      ++head;
      while(std::isalnum(*head) || *head == '_') ++head;

      std::string value(begin, head);

      // handle special ID's
      if (value.size() == 1)
      {
        if (value[0] == 'z')
        {
          tokens.emplace_back(Token::Type::Z, value);
          continue;
        }
        if (value[0] == 'i')
        {
          tokens.emplace_back(Token::Type::I, value);
          continue;
        }
      }

      tokens.emplace_back(Token::Type::ID, value);
      continue;
    }

    tokens.emplace_back(Token::Type::NONE);
    break;
  }

  tokens.emplace_back(Token::Type::EOD, eos, eos);
}

class Parser
{
public:
  explicit Parser(std::string const & input) : lexer(input) {}

  void parse(Expression & expression)
  {
    lexer.tokenize();
    head = lexer.begin();
    auto node = parseExpression(expression);
    expect(Lexer::Token::Type::EOD);
    expression.set_root(node);
  }

private:
  Lexer lexer;
  Lexer::TokenIterator head;

  Lexer::TokenIterator current;

  bool accept(Lexer::Token::Type type)
  {
    return (head->type == type) ? current = head++, true : false;
  }

  void expect(Lexer::Token::Type type)
  {
    if (head->type == type) ++head;
    else throw std::invalid_argument("syntax error");
  }

  /*
    Grammar:

    E -> '-'? S ( ('+'|'-') S )*
    S -> F ( ('*'|'/') F )*
    F -> A ( '^' A )?
    A -> id? '(' E ')'
    A -> 'z'
    A -> real
    A -> 'i'
  */

  Expression::Node * parseExpression(Expression & expression)
  {
    bool neg = accept(Lexer::Token::Type::ADD) && (current->value[0] == '-');
    auto node = parseSummand(expression);
    if (neg)
      node = expression.new_Node(node, nullptr,
          [](complex const & a, complex const &) { return -a; });
    while (accept(Lexer::Token::Type::ADD))
    {
      char op = current->value[0];
      auto node2 = parseSummand(expression);
      node = expression.new_Node(node, node2, (op == '+') ?
          [](complex const & a, complex const & b) { return a + b; } :
          [](complex const & a, complex const & b) { return a - b; });
    }
    return node;
  }

  Expression::Node * parseSummand(Expression & expression)
  {
    auto node = parseFactor(expression);
    while (accept(Lexer::Token::Type::MUL))
    {
      char op = current->value[0];
      auto node2 = parseFactor(expression);
      node = expression.new_Node(node, node2, (op == '*') ?
          [](complex const & a, complex const & b) { return a*b; } :
          [](complex const & a, complex const & b) { return a/b; });
    }
    return node;
  }

  Expression::Node * parseFactor(Expression & expression)
  {
    auto node = parseAtomic(expression);
    if (accept(Lexer::Token::Type::POW))
    {
      auto node2 = parseAtomic(expression);
      node = expression.new_Node(node, node2, [](complex const & a, complex const & b) { return std::pow(a, b); });
    }
    return node;
  }

  Expression::Node * parseAtomic(Expression & expression)
  {
    if (accept(Lexer::Token::Type::ID))
    {
      auto it = Expression::fun.find(current->value);
      if (it == Expression::fun.end())
        throw std::invalid_argument(std::string("unknown identifier '") + current->value + "'");
      expect(Lexer::Token::Type::LP);
      auto node = parseExpression(expression);
      expect(Lexer::Token::Type::RP);
      return expression.new_Node(node, nullptr, it->second);
    }

    if (accept(Lexer::Token::Type::LP))
    {
      auto node = parseExpression(expression);
      expect(Lexer::Token::Type::RP);
      return node;
    }

    if (accept(Lexer::Token::Type::REAL))
    {
      complex c(std::stod(current->value));
      return expression.new_Node(nullptr, nullptr, [c](complex const &, complex const &) { return c; });
    }

    if (accept(Lexer::Token::Type::I))
    {
      complex c(0.0, 1.0);
      return expression.new_Node(nullptr, nullptr, [c](complex const &, complex const &) { return c; });
    }

    if (accept(Lexer::Token::Type::Z))
    {
      return expression.new_Node(nullptr, nullptr, [](complex const & a, complex const &) { return a; });
    }

    throw std::invalid_argument("syntax error");
  }
};

} // namespace

void Function::fromFormula(std::string const & formula)
{
  Parser parser(formula);
  Expression new_expression;
  parser.parse(new_expression);
  expression = std::move(new_expression);
}
