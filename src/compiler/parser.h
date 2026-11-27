#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "compiler/lexer.h" // Token, TokenType

// Часть 1. Дерево программы
// Какой узел лежит в указателе - через dynamic_cast

namespace ast {

struct Expr {
    int line = 0;
    virtual ~Expr() = default;
};

struct Stmt {
    int line = 0;
    virtual ~Stmt() = default;
};

struct TypeExpr {
    int line = 0;
    virtual ~TypeExpr() = default;
};

using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;
using TypePtr = std::unique_ptr<TypeExpr>;

// типы

// int64, float64, ...  (потом добавятся массивы и единицы измерения - новыми
// наследниками TypeExpr)
struct NamedType : TypeExpr {
    std::string name;
};

// выражения

// 42
struct IntLit : Expr {
    int64_t value = 0;
};

// x
struct Ident : Expr {
    std::string name;
};

// -x, !x
struct Unary : Expr {
    TokenType op{};
    ExprPtr operand;
};

// a + b, x < y
struct Binary : Expr {
    TokenType op{};
    ExprPtr left;
    ExprPtr right;
};

// print(x), add(1, 2)
struct Call : Expr {
    ExprPtr
        callee; // что вызываем; пока всегда Ident, потом может быть p.method
    std::vector<ExprPtr> args;
};

// инструкции

// var x: int32 = 10 - тип или значение может отсутствовать, но не оба
struct VarDecl : Stmt {
    std::string name;
    TypePtr type;  // может быть nullptr, если есть значение
    ExprPtr value; // может быть nullptr, если есть тип
};

// x := 10
struct Define : Stmt {
    std::string name;
    ExprPtr value;
};

// x = 10
struct Assign : Stmt {
    ExprPtr target; // пока всегда Ident, потом arr[i], p.x
    ExprPtr value;
};

// print(x)
struct ExprStmt : Stmt {
    ExprPtr expr;
};

// { ... }
struct Block : Stmt {
    std::vector<StmtPtr> body;
};

// if cond { } else { } elseBranch - Block, If (else if) или nullptr
struct If : Stmt {
    ExprPtr cond;
    std::unique_ptr<Block> then;
    StmtPtr elseBranch;
};

// for cond { } если - init и step = nullptr
// for init; cond; step { }
struct For : Stmt {
    StmtPtr init; // может быть nullptr
    ExprPtr cond;
    StmtPtr step; // может быть nullptr
    std::unique_ptr<Block> body;
};

// return x / return
struct Return : Stmt {
    ExprPtr value; // может быть nullptr
};

// верхний уровень

struct Param {
    std::string name;
    TypePtr type;
    int line = 0;
};

struct Func {
    std::string name;
    std::vector<Param> params;
    TypePtr result; // может быть nullptr - функция ничего не возвращает
    std::unique_ptr<Block> body;
    int line = 0;
};

struct Module {
    std::vector<std::unique_ptr<Func>> funcs;
};

} // namespace ast

// Часть 2. Парсер: токены → дерево

class ParseError : public std::runtime_error {
  public:
    int line;
    ParseError(const std::string &message, int line)
        : std::runtime_error("line " + std::to_string(line) + ": " + message),
          line(line) {}
};

class Parser {
  public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

    // весь файл; при первой ошибке бросает ParseError
    ast::Module parse();

  private:
    std::vector<Token> tokens;
    size_t pos = 0;

    // работа с токенами
    const Token &peek() const { return tokens[pos]; } // текущий токен
    bool check(TokenType t) const {
        return peek().type == t;
    } // текущий такого типа?
    const Token &advance() { return tokens[pos++]; } // взять и сдвинуться
    bool match(TokenType t); // если такой - съесть и true
    const Token &
    expect(TokenType t,
           const std::string &what); // обязан быть, иначе ParseError

    // верхний уровень
    std::unique_ptr<ast::Func> parseFunc();
    ast::Param parseParam();
    ast::TypePtr parseType();

    // инструкции
    std::unique_ptr<ast::Block> parseBlock();
    ast::StmtPtr parseStatement();
    ast::StmtPtr parseSimple();
    ast::StmtPtr parseVarDecl();
    ast::StmtPtr parseIf();
    ast::StmtPtr parseFor();
    ast::StmtPtr parseReturn();

    // выражения, от слабого приоритета к сильному
    ast::ExprPtr parseExpr();
    ast::ExprPtr parseOr();
    ast::ExprPtr parseAnd();
    ast::ExprPtr parseCmp();
    ast::ExprPtr parseAdd();
    ast::ExprPtr parseMul();
    ast::ExprPtr parseUnary();
    ast::ExprPtr parsePostfix();
    ast::ExprPtr parsePrimary();
};
