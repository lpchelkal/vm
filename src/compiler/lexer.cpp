#include "lexer.h"

#include <algorithm>

// Проверка: лексер на программе ниже должен выдать ровно эти токены
bool testLexer() {
    std::string src = R"(// пример программы
const N = 10

func fact(n: int32) int32 {
    if n <= 1 {
        return 1
    }
    return n * fact(n - 1)
}

func main() {
    var arr: [N]float64
    s := 0
    for i := 0; i < N; i++ {
        if i % 2 == 0 && !(i > 7) || i != 3 {
            continue
        }
        arr[i] = 2.5 / 1.0 - 0.5
        s = s + i
        if s >= 20 {
            break
        } else {
            s--
        }
    }
    c := 'x'
    ok := true
    p := nil
    print("done", fact(5), s, c, ok, false, p)
    p.x = 1
}
)";

    using T = TokenType;
    std::vector<Token> expected = {
        {T::Const, "const", 2},
        {T::Ident, "N", 2},
        {T::Assign, "=", 2},
        {T::Int, "10", 2},
        {T::Semicolon, "\n", 2},
        {T::Func, "func", 4},
        {T::Ident, "fact", 4},
        {T::LParen, "(", 4},
        {T::Ident, "n", 4},
        {T::Colon, ":", 4},
        {T::Ident, "int32", 4},
        {T::RParen, ")", 4},
        {T::Ident, "int32", 4},
        {T::LBrace, "{", 4},
        {T::If, "if", 5},
        {T::Ident, "n", 5},
        {T::Leq, "<=", 5},
        {T::Int, "1", 5},
        {T::LBrace, "{", 5},
        {T::Return, "return", 6},
        {T::Int, "1", 6},
        {T::Semicolon, "\n", 6},
        {T::RBrace, "}", 7},
        {T::Semicolon, "\n", 7},
        {T::Return, "return", 8},
        {T::Ident, "n", 8},
        {T::Mul, "*", 8},
        {T::Ident, "fact", 8},
        {T::LParen, "(", 8},
        {T::Ident, "n", 8},
        {T::Sub, "-", 8},
        {T::Int, "1", 8},
        {T::RParen, ")", 8},
        {T::Semicolon, "\n", 8},
        {T::RBrace, "}", 9},
        {T::Semicolon, "\n", 9},
        {T::Func, "func", 11},
        {T::Ident, "main", 11},
        {T::LParen, "(", 11},
        {T::RParen, ")", 11},
        {T::LBrace, "{", 11},
        {T::Var, "var", 12},
        {T::Ident, "arr", 12},
        {T::Colon, ":", 12},
        {T::LBrack, "[", 12},
        {T::Ident, "N", 12},
        {T::RBrack, "]", 12},
        {T::Ident, "float64", 12},
        {T::Semicolon, "\n", 12},
        {T::Ident, "s", 13},
        {T::Define, ":=", 13},
        {T::Int, "0", 13},
        {T::Semicolon, "\n", 13},
        {T::For, "for", 14},
        {T::Ident, "i", 14},
        {T::Define, ":=", 14},
        {T::Int, "0", 14},
        {T::Semicolon, ";", 14},
        {T::Ident, "i", 14},
        {T::Lss, "<", 14},
        {T::Ident, "N", 14},
        {T::Semicolon, ";", 14},
        {T::Ident, "i", 14},
        {T::Inc, "++", 14},
        {T::LBrace, "{", 14},
        {T::If, "if", 15},
        {T::Ident, "i", 15},
        {T::Rem, "%", 15},
        {T::Int, "2", 15},
        {T::Eql, "==", 15},
        {T::Int, "0", 15},
        {T::LAnd, "&&", 15},
        {T::Not, "!", 15},
        {T::LParen, "(", 15},
        {T::Ident, "i", 15},
        {T::Gtr, ">", 15},
        {T::Int, "7", 15},
        {T::RParen, ")", 15},
        {T::LOr, "||", 15},
        {T::Ident, "i", 15},
        {T::Neq, "!=", 15},
        {T::Int, "3", 15},
        {T::LBrace, "{", 15},
        {T::Continue, "continue", 16},
        {T::Semicolon, "\n", 16},
        {T::RBrace, "}", 17},
        {T::Semicolon, "\n", 17},
        {T::Ident, "arr", 18},
        {T::LBrack, "[", 18},
        {T::Ident, "i", 18},
        {T::RBrack, "]", 18},
        {T::Assign, "=", 18},
        {T::Float, "2.5", 18},
        {T::Quo, "/", 18},
        {T::Float, "1.0", 18},
        {T::Sub, "-", 18},
        {T::Float, "0.5", 18},
        {T::Semicolon, "\n", 18},
        {T::Ident, "s", 19},
        {T::Assign, "=", 19},
        {T::Ident, "s", 19},
        {T::Add, "+", 19},
        {T::Ident, "i", 19},
        {T::Semicolon, "\n", 19},
        {T::If, "if", 20},
        {T::Ident, "s", 20},
        {T::Geq, ">=", 20},
        {T::Int, "20", 20},
        {T::LBrace, "{", 20},
        {T::Break, "break", 21},
        {T::Semicolon, "\n", 21},
        {T::RBrace, "}", 22},
        {T::Else, "else", 22},
        {T::LBrace, "{", 22},
        {T::Ident, "s", 23},
        {T::Dec, "--", 23},
        {T::Semicolon, "\n", 23},
        {T::RBrace, "}", 24},
        {T::Semicolon, "\n", 24},
        {T::RBrace, "}", 25},
        {T::Semicolon, "\n", 25},
        {T::Ident, "c", 26},
        {T::Define, ":=", 26},
        {T::Char, "x", 26},
        {T::Semicolon, "\n", 26},
        {T::Ident, "ok", 27},
        {T::Define, ":=", 27},
        {T::True, "true", 27},
        {T::Semicolon, "\n", 27},
        {T::Ident, "p", 28},
        {T::Define, ":=", 28},
        {T::Nil, "nil", 28},
        {T::Semicolon, "\n", 28},
        {T::Ident, "print", 29},
        {T::LParen, "(", 29},
        {T::String, "done", 29},
        {T::Comma, ",", 29},
        {T::Ident, "fact", 29},
        {T::LParen, "(", 29},
        {T::Int, "5", 29},
        {T::RParen, ")", 29},
        {T::Comma, ",", 29},
        {T::Ident, "s", 29},
        {T::Comma, ",", 29},
        {T::Ident, "c", 29},
        {T::Comma, ",", 29},
        {T::Ident, "ok", 29},
        {T::Comma, ",", 29},
        {T::False, "false", 29},
        {T::Comma, ",", 29},
        {T::Ident, "p", 29},
        {T::RParen, ")", 29},
        {T::Semicolon, "\n", 29},
        {T::Ident, "p", 30},
        {T::Period, ".", 30},
        {T::Ident, "x", 30},
        {T::Assign, "=", 30},
        {T::Int, "1", 30},
        {T::Semicolon, "\n", 30},
        {T::RBrace, "}", 31},
        {T::Semicolon, "\n", 31},
        {T::Eof, "", 32},
    };

    return std::ranges::equal(
        Lexer(src).tokenize(), expected, [](const Token& a, const Token& b) {
            return a.type == b.type && a.text == b.text && a.line == b.line;
        });
}
