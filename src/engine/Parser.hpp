#pragma once

#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>

#include "Polynomial.hpp"

class Lexer
{
public:
    explicit Lexer(std::string const & input) :
        head(input.cbegin()),
        current{Token::Type::UNKNOWN, head, head}
    {
        advance();
    }

    struct Token
    {
        enum class Type
        {
            PLUS, MINUS, AST, HAT,
            LPAREN, RPAREN,
            Z, W, I,
            INT, FLOAT,
            EOD, UNKNOWN
        };

        Type type;
        std::string::const_iterator first;
        std::string::const_iterator last;

        friend std::ostream & operator<<(std::ostream & out, Token const & t)
        {
            out << '\"';
            std::copy(t.first, t.last, std::ostream_iterator<std::string::const_iterator::value_type>(out, ""));
            out << "\" of type " << static_cast<std::uint64_t>(t.type);
        }
    };

    Token const & currentToken() const { return current; }

    void advance()
    {
        // skip whitespaces
        while (std::isspace(*head)) ++head;
        auto tokenStart = head;
        Token::Type type = Token::Type::UNKNOWN;

        if (*head == '$')
        {
            current = {Token::Type::EOD, head, head};
            return;
        }

        // one-character
        switch(*head)
        {
        case '+':
            type = Token::Type::PLUS;
            break;
        case '-':
            type = Token::Type::MINUS;
            break;
        case '*':
            type = Token::Type::AST;
            break;
        case '^':
            type = Token::Type::HAT;
            break;
        case '(':
            type = Token::Type::LPAREN;
            break;
        case ')':
            type = Token::Type::RPAREN;
            break;
        case 'z':
            type = Token::Type::Z;
            break;
        case 'w':
            type = Token::Type::W;
            break;
        case 'i':
            type = Token::Type::I;
            break;
        }

        if (type != Token::Type::UNKNOWN)
        {
            ++head;
            current = {type, tokenStart, head};
            return;
        }

        // number: int (ddd) or float (d+.d*['e'['+'|'-']d+]), no sign in front, decimal only
        if (std::isdigit(*head))
        {
            ++head;
            while(std::isdigit(*head)) ++head;
            if (*head != '.') // int
            {
                current = {Token::Type::INT, tokenStart, head};
                return;
            }

            ++head; // eat '.'
            while(std::isdigit(*head)) ++head;
            if (*head != 'e') // float, short form
            {
                current = {Token::Type::FLOAT, tokenStart, head};
                return;
            }

            ++head; // eat 'e'
            if (*head == '+' || *head == '-') ++head;
            if (std::isdigit(*head))
            {
                ++head;
                while(std::isdigit(*head)) ++head;
                current = {Token::Type::FLOAT, tokenStart, head};
                return;
            }
        }

        current = {Token::Type::UNKNOWN, tokenStart, head};
    }

private:
    std::string::const_iterator head;

    Token current;
};

template <typename ParserActions>
class Parser
{
public:
    template <typename InputIt, typename ... Args>
    explicit Parser(InputIt first, InputIt last, Args && ... args) :
        lexerInput(std::string(first, last) + '$'), // add sentinel
        lexer(lexerInput),
        parserActions(std::forward<Args>(args)...)
    {}

    using ResultType = typename ParserActions::ResultType;
    using IntegerType = typename ParserActions::IntegerType;
    using FloatType = typename ParserActions::FloatType;

    ResultType parse()
    {
        auto result = parseExpression();
        expect(Lexer::Token::Type::EOD);
        return result;
    }

private:
    std::string const lexerInput;
    Lexer lexer;
    Lexer::Token currentToken;

    ParserActions parserActions;

    bool accept(Lexer::Token::Type type)
    {
        currentToken = lexer.currentToken();
        if (currentToken.type == type)
        {
            lexer.advance();
            return true;
        }

        return false;
    }

    void expect(Lexer::Token::Type type)
    {
        currentToken = lexer.currentToken();
        if (currentToken.type == type)
        {
            lexer.advance();
            return;
        }

        syntaxError();
    }

    void syntaxError()
    {
        // TODO: prepare nice message based on current token
        throw std::invalid_argument("syntax error");
    }

    //  Grammar:
    //  E -> '-' ? S ( ( '+' | '-' ) S ) *
    //  S -> F ( '*' F ) *
    //  F -> A ( '^' int ) ?
    //  A -> '(' E ')' | 'z' | 'w' | 'i' | float

    ResultType parseExpression()
    {
        bool neg = accept(Lexer::Token::Type::MINUS);
        auto resultFirst = parseSummand();
        if (neg)
            resultFirst = parserActions.onNeg(resultFirst);

        while (accept(Lexer::Token::Type::PLUS) || accept(Lexer::Token::Type::MINUS))
        {
            auto tokenType = currentToken.type;
            auto resultNext = parseSummand();
            switch (tokenType)
            {
            case Lexer::Token::Type::PLUS:
                resultFirst = parserActions.onAdd(resultFirst, resultNext);
                break;
            case Lexer::Token::Type::MINUS:
                resultFirst = parserActions.onSub(resultFirst, resultNext);
                break;
            default:
                // should not happen
                throw std::logic_error("Internal error.");
            }
        }

        return resultFirst;
    }

    ResultType parseSummand()
    {
        auto resultFirst = parseFactor();
        while (accept(Lexer::Token::Type::AST))
        {
            auto resultNext = parseFactor();
            resultFirst = parserActions.onMul(resultFirst, resultNext);
        }

        return resultFirst;
    }

    ResultType parseFactor()
    {
        auto result = parseAtomic();
        if (accept(Lexer::Token::Type::HAT))
        {
            expect(Lexer::Token::Type::INT);
            result = parserActions.onPow(result, getInteger(currentToken));
        }

        return result;
    }

    ResultType parseAtomic()
    {
        if (accept(Lexer::Token::Type::LPAREN))
        {
            auto result = parseExpression();
            expect(Lexer::Token::Type::RPAREN);
            return result;
        }

        if (accept(Lexer::Token::Type::FLOAT) || accept(Lexer::Token::Type::INT))
        {
            return parserActions.onFloat(getFloat(currentToken));
        }

        if (accept(Lexer::Token::Type::I))
        {
            return parserActions.onI();
        }

        if (accept(Lexer::Token::Type::Z))
        {
            return parserActions.onZ();
        }

        if (accept(Lexer::Token::Type::W))
        {
            return parserActions.onW();
        }

        syntaxError();
        throw 0; // unreachable, to suppress warning
    }

    IntegerType getInteger(Lexer::Token const & token)
    {
        // TODO: use from_chars when available
        return static_cast<IntegerType>(std::stol(std::string(currentToken.first, currentToken.last)));
    }

    FloatType getFloat(Lexer::Token const & token)
    {
        // TODO: use from_chars when available
        return static_cast<FloatType>(std::stod(std::string(currentToken.first, currentToken.last)));
    }
};
