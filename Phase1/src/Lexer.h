#ifndef LEXER_H // conditional compilations(checks whether a macro is not defined)
#define LEXER_H

#include "llvm/ADT/StringRef.h"        // encapsulates a pointer to a C string and its length
#include "llvm/Support/MemoryBuffer.h" // read-only access to a block of memory, filled with the content of a file

class Lexer;

class Token
{
    friend class Lexer; // Lexer can access private and protected members of Token

public:
    enum TokenKind : unsigned short
    {
        eoi,     // end of input
        unknown, // in case of error at the lexical level
        ident,
        number,
        equal,
        comma,
        colon,
        semicolon,
        hat,
        plus,
        minus,
        star,
        slash,
        mod,
        gr,
        ge,
        ls,
        le,
        nq,
        eq,
        KW_and,
        KW_or,
        minus_equal,
        plus_equal,
        star_equal,
        slash_equal,
        equal_equal,
        l_paren,
        r_paren,
        KW_int,
        KW_if,
        KW_else,
        KW_elif,
        KW_begin,
        KW_end,
        KW_loopc,
    };

private:
    TokenKind Kind;
    llvm::StringRef Text; // points to the start of the text of the token

public:
    TokenKind getKind() const { return Kind; }
    llvm::StringRef getText() const { return Text; }

    // to test if the token is of a certain kind
    bool is(TokenKind K) const { return Kind == K; }
    bool isOneOf(TokenKind K1, TokenKind K2) const
    {
        return is(K1) || is(K2);
    }
    template <typename... Ts>
    bool isOneOf(TokenKind K1, TokenKind K2, Ts... Ks)
        const { return is(K1) || isOneOf(K2, Ks...); }
};

class Lexer
{
    const char *BufferStart; // pointer to the beginning of the input
    const char *BufferPtr;   // pointer to the next unprocessed character

public:
    Lexer(const llvm::StringRef &Buffer)
    {
        BufferStart = Buffer.begin();
        BufferPtr = BufferStart;
    }

    void next(Token &token); // return the next token

private:
    void formToken(Token &Result, const char *TokEnd, Token::TokenKind Kind);
};
#endif