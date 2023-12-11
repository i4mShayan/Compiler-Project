#include "Lexer.h"

// classifying characters
namespace charinfo
{
    // ignore whitespaces
    LLVM_READNONE inline bool isWhitespace(char c)
    {
        return c == ' ' || c == '\t' || c == '\f' || c == '\v' ||
               c == '\r' || c == '\n';
    }

    LLVM_READNONE inline bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    LLVM_READNONE inline bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
}

void Lexer::next(Token &token)
{
    while (*BufferPtr && charinfo::isWhitespace(*BufferPtr))
    {
        ++BufferPtr;
    }
    // make sure we didn't reach the end of input
    if (!*BufferPtr)
    {
        token.Kind = Token::eoi;
        return;
    }
    // collect characters and check for keywords or ident
    if (charinfo::isLetter(*BufferPtr))
    {
        const char *end = BufferPtr + 1;
        while (charinfo::isLetter(*end))
            ++end;
        llvm::StringRef Name(BufferPtr, end - BufferPtr);
        Token::TokenKind kind;
        if (Name == "int")
            kind = Token::KW_int;
        else if (Name == "if")
            kind = Token::KW_if;
        else if (Name == "else")
            kind = Token::KW_else;
        else if (Name == "elif")
            kind = Token::KW_elif;
        else if (Name == "begin")
            kind = Token::KW_begin;
        else if (Name == "end")
            kind = Token::KW_end;
        else if (Name == "loopc")
            kind = Token::KW_loopc;
        else if (Name == "and")
            kind = Token::KW_and;
        else if (Name == "or")
            kind = Token::KW_or;
        else 
            kind = Token::ident;
        // generate the token
        formToken(token, end, kind);
        return;
    }
    // check for numbers
    else if (charinfo::isDigit(*BufferPtr))
    {
        const char *end = BufferPtr + 1;
        while (charinfo::isDigit(*end))
            ++end;
        formToken(token, end, Token::number);
        return;
    }
    else
    {

        switch (*BufferPtr)
        {
        case '>':
            if (*(BufferPtr + 1) == '=')
            {
                formToken(token, BufferPtr + 2, Token::ge);
                return;
            }
            else
            {
                formToken(token, BufferPtr + 1, Token::gr);
                return;
            }
            break;
        case '<':
            if (*(BufferPtr + 1) == '=')
            {
                formToken(token, BufferPtr + 2, Token::le);
                return;
            }
            else
            {
                formToken(token, BufferPtr + 1, Token::ls);
                return;
            }
            break;
        case '=':
            if (*(BufferPtr + 1) == '=')
            {
                formToken(token, BufferPtr + 2, Token::equal_equal); // equal
                return;
            }
            else
            {
                formToken(token, BufferPtr + 1, Token::equal);
                return;
            }
            break;
        case '!':
            if (*(BufferPtr + 1) == '=')
            {
                formToken(token, BufferPtr + 2, Token::nq);
                return;
            }
            break;
        case '+':
            if (*(BufferPtr + 1) == '=')
            {
                formToken(token, BufferPtr + 2, Token::plus_equal);
                return;
            }
            else
            {
                formToken(token, BufferPtr + 1, Token::plus);
                return;
            }
            break;
        case '-':
            if (*(BufferPtr + 1) == '=')
            {
                formToken(token, BufferPtr + 2, Token::minus_equal);
                return;
            }
            else
            {
                formToken(token, BufferPtr + 1, Token::minus);
                return;
            }
            break;

        case '*':
            if (*(BufferPtr + 1) == '=')
            {
                formToken(token, BufferPtr + 2, Token::star_equal);
                return;
            }
            else
            {
                formToken(token, BufferPtr + 1, Token::star);
                return;
            }
            break;
        case '/':
            if (*(BufferPtr + 1) == '=')
            {
                formToken(token, BufferPtr + 2, Token::slash_equal);
                return;
            }
            else
            {
                formToken(token, BufferPtr + 1, Token::slash);
                return;
            }
            break;

        case '(':
            formToken(token, BufferPtr + 1, Token::l_paren);
            return;
            break;
        case ')':
            formToken(token, BufferPtr + 1, Token::r_paren);
            return;
            break;
        
        case ':':
            formToken(token, BufferPtr + 1, Token::colon);
            return;
            break;
        case ';':
            formToken(token, BufferPtr + 1, Token::semicolon);
            return;
            break;
        case '^':
             formToken(token, BufferPtr +1 , Token::hat);
             return ;
        default:
            formToken(token, BufferPtr + 1, Token::unknown);
            break;
        }
    }
}

void Lexer::formToken(Token &Tok, const char *TokEnd,
                      Token::TokenKind Kind)
{
    Tok.Kind = Kind;
    Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
    BufferPtr = TokEnd;
}