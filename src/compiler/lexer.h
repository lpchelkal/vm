#pragma once

#include <string>
#include <unordered_map>
#include <vector>

enum class TokenType {
    // служебные: Illegal — непонятный символ, Eof — конец текста, Comment — //
    // комментарий
    Illegal,
    Eof,
    Comment,

    // со значением в text: имя, 42, 2.5, 'a', "hello"
    Ident,
    Int,
    Float,
    Char,
    String,

    // + - * / % ++ --
    Add,
    Sub,
    Mul,
    Quo,
    Rem,
    Inc,
    Dec,

    // && || !
    LAnd,
    LOr,
    Not,

    // == != < > <= >=
    Eql,
    Neq,
    Lss,
    Gtr,
    Leq,
    Geq,

    // =  :=
    Assign,
    Define,

    // ( ) [ ] { }
    LParen,
    RParen,
    LBrack,
    RBrack,
    LBrace,
    RBrace,

    // , . ; :
    Comma,
    Period,
    Semicolon,
    Colon,

    // break const continue else for func if return var true false nil
    Break,
    Const,
    Continue,
    Else,
    For,
    Func,
    If,
    Return,
    Var,
    True,
    False,
    Nil,
};

inline const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"break", TokenType::Break},
    {"const", TokenType::Const},
    {"continue", TokenType::Continue},
    {"else", TokenType::Else},
    {"for", TokenType::For},
    {"func", TokenType::Func},
    {"if", TokenType::If},
    {"return", TokenType::Return},
    {"var", TokenType::Var},
    {"true", TokenType::True},
    {"false", TokenType::False},
    {"nil", TokenType::Nil},
};

struct Token {
    TokenType type;
    std::string text;

    int line;
};

class Lexer {
    std::string src;
    size_t pos = 0;
    int line = 1;

    // текущий символ, '\0' - текст кончился
    char peek() const {
        return pos < src.size() ? src[pos] : '\0';
    }
    // взять текущий символ и сдвинуться
    char advance() {
        return src[pos++];
    }

public:
    explicit Lexer(std::string source)
        : src(std::move(source)) {
    }

    // следующий токен; в конце текста - Eof
    Token next() {
        return {TokenType::Eof, "", line};
    }

    // все токены по порядку, последний - Eof
    std::vector<Token> tokenize() {
        std::vector<Token> res;
        do {
            res.push_back(next());
        } while (res.back().type != TokenType::Eof);
        return res;
    }
};
