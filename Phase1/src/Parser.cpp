#include "Parser.h"

// main point is that the whole input has been consumed
AST *Parser::parse()
{
    AST *Res = parseGSM();
    return Res;
}


AST *Parser::parseGSM()
{
    llvm::SmallVector<Expr *> Exprs;
    while (!Tok.is(Token::eoi))
    {
        switch (Tok.getKind())
        {
            case Token::KW_int:
            {
                Expr *d;
                d = parseDecl();
                if (d)
                    Exprs.push_back(d);
                else    
                    goto _error2;
                break;
            }
            case Token::ident
            {
                Expr *a;
                a = parseAssign();
                if (!Tok.is(Token::semicolon))
                {
                    error();
                    goto _error2;
                }
                if (a)
                    Exprs.push_back(a);
                else
                    goto _error2;
                break;
            }
            case Token::KW_if:
            {
                Expr *a;
                a = parseIf();
                if (a)
                    Exprs.push_back(a);
                else
                    goto _error2;
                break;
            }
            case Token::KW_loopc:
            {
                Expr *a;
                a = parseLoopc();
                if (a)
                    Exprs.push_back(a);
                else
                    goto _error2;
                break;
            }
            default:
                goto _error2;
                break;
        }
        advance();
    }
    return new GSM(Exprs);
_error2:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}


Expr *Parser::parseDec()
{
    llvm::SmallVector<Expr *> Exprs;
    llvm::SmallVector<llvm::StringRef, 8> Vars;

    if (expect(Token::KW_int))
        goto _error;

    advance();

    if (expect(Token::ident))
        goto _error;
    Vars.push_back(Tok.getText());
    advance();

    while (Tok.is(Token::comma))
    {
        advance();
        if (expect(Token::ident))
            goto _error;
        Vars.push_back(Tok.getText());
        advance();
    }

    if (Tok.is(Token::equal))
    {
        advance();
        Expr *E;
        E = parseExpr();
        if (E)
            Exprs.push_back(E);
        else
            goto _error;
        while (Tok.is(Token::comma))
        {
            advance();
            E = parseExpr();
            if (E)
                Exprs.push_back(E);
            else
                goto _error;
        }
    }

    if (expect(Token::semicolon))
        goto _error;

    return new Declaration(Vars, Exprs);
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}


Expr *Parser::parseAssign()
{
    Expr *E;
    Final *F;
    F = (Final *)(parseFinal());

    switch (Tok.getKind())
    {
        case Token::equal:
        {
            advance();
            E = parseExpr();
            if (E)
                return new Assignment(F, E);
            else
                goto _error;
            break;
        }
        case Token::plus_equal:
        {
            advance();
            E = parseExpr();
            if (E)
                return new Assignment(F, new BinaryOp(Operator::plus, F, E));
            else
                goto _error;
            break;
        }
        case Token::minus_equal:
        {
            advance();
            E = parseExpr();
            if (E)
                return new Assignment(F, new BinaryOp(Operator::minus, F, E));
            else
                goto _error;
            break;
        }
        case Token::star_equal:
        {
            advance();
            E = parseExpr();
            if (E)
                return new Assignment(F, new BinaryOp(Operator::star, F, E));
            else
                goto _error;
            break;
        }
        case Token::slash_equal:
        {
            advance();
            E = parseExpr();
            if (E)
                return new Assignment(F, new BinaryOp(Operator::slash, F, E));
            else
                goto _error;
            break;
        }
        default:
            goto _error;
            break;
    }

}


Expr *Parser::parseExpr()
{
    Expr *Left = parseTerm();
    while (Tok.isOneOf(Token::plus, Token::minus))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::plus) ? BinaryOp::Plus : BinaryOp::Minus;
        advance();
        Expr *Right = parseTerm();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}


Expr *Parser::parseTerm()
{
    Expr *Left = parseFactor();
    while (Tok.isOneOf(Token::star, Token::slash))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::star) ? BinaryOp::Mul : BinaryOp::Div;
        advance();
        Expr *Right = parseFactor();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}


Expr *Parser::parseFactor()
{
    Expr *Left = parseFinal();
    while (Tok.is(Token::hat))
    {
        advance();
        Expr *Right = parseFinal();
        Left = new BinaryOp(BinaryOp::Pow, Left, Right);
    }
    return Left;
}


Expr *Parser::parseFinal()
{
    Expr *Res;
    switch (Tok.getKind())
    {
    case Token::number:
        Res = new Final(Final::Number, Tok.getText());
        advance();
        break;
    case Token::ident:
        Res = new Final(Final::Ident, Tok.getText());
        advance();
        break;
    case Token::l_paren:
        advance();
        Res = parseExpr();
        if (!consume(Token::r_paren))
            break;
    default: // error handling
        if (!Res)
            error();
        while (!Tok.isOneOf(Token::r_paren, Token::star, Token::plus, Token::minus, Token::slash, Token::eoi))
            advance();
        break;
    }
    return Res;
}


Expr *Parser::parseConditions()
{
    Expr *Left = parseCondition();
    while (Tok.isOneOf(Token::and, Token::or))
    {
        LogicalOp::Operator Op =
            Tok.is(Token::and) ? LogicalOp::And : LogicalOp::Or;
        advance();
        Expr *Right = parseCondition();
        Left = new LogicalOp(Op, Left, Right);
    }
    return Left;
}


Expr *Parser::parseCondition()
{
    Expr *Left = parseExpr();
    BinaryOp::Operator Op;
    switch (Tok.getKind())
    {
        case Token::equal_equal:
            Op = BinaryOp::Equal;
            break;
        case Token::nq:
            Op = BinaryOp::NotEqual;
            break;
        case Token::ls:
            Op = BinaryOp::Less;
            break;
        case Token::gr:
            Op = BinaryOp::Greater;
            break;
        case Token::le:
            Op = BinaryOp::LessEqual;
            break;
        case Token::ge:
            Op = BinaryOp::GreaterEqual;
            break;
        default:
            error();
            return nullptr;
    }
    advance();
    Expr *Right = parseExpr();
    return new BinaryOp(Op, Left, Right);
}

// IF → "if" CONDITIONS ":" "begin" (ASSIGN)* "end" (ELIF)* (ELSE)?
Expr *Parser::parseIf()
{
    Condition *Cond;
    llvm::SmallVector<Expr *> Assigns;
    llvm::SmallVector<ELIF *> Elifs;
    ELSE *Else;

    if (expect(Token::KW_if))
        goto _error;
    advance();

    Cond = parseConditions();
    if (expect(Token::colon))
        goto _error;
    advance();

    if (expect(Token::KW_begin))
        goto _error;
    advance();

    while (!Tok.is(Token::KW_end))
    {
        Expr *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();
    
    while (Tok.is(Token::KW_elif))
    {
        // advance(); It is not necessary to advance here because parseElif() will do it
        Expr *elif;
        elif = parseElif();
        if (elif)
            Elifs.push_back(elif);
        else
            goto _error;
    }

    if (Tok.is(Token::KW_else))
    {
        // advance(); It is not necessary to advance here because parseElse() will do it
        Else = parseElse();
        if (!Else)
            goto _error;
    }

    return new If(Cond, Assigns, Elifs, Else);

}

// ELIF → "elif" CONDITIONS ":" "begin" (ASSIGN)* "end"
Expr *Parser::parseElif()
{
    Condition *Cond;
    llvm::SmallVector<Expr *> Assigns;

    if (expect(Token::KW_elif))
        goto _error;
    advance();

    Cond = parseConditions();
    if (expect(Token::colon))
        goto _error;
    advance();

    if (expect(Token::KW_begin))
        goto _error;
    advance();

    while (!Tok.is(Token::KW_end))
    {
        Expr *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    return new ELIF(Cond, Assigns);

}

// ELSE→ "else" ":" "begin" (ASSIGN)* "end"

Expr *Parser::parseElse()
{
    llvm::SmallVector<Expr *> Assigns;

    if (expect(Token::KW_else))
        goto _error;
    advance();

    if (expect(Token::colon))
        goto _error;
    advance();

    if (expect(Token::KW_begin))
        goto _error;
    advance();

    while (!Tok.is(Token::KW_end))
    {
        Expr *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    return new ELSE(Assigns);

}

// LOOPC → "loopc" CONDITIONS ":" "begin" (ASSIGN)* "end"
Expr *Parser::parseLoopc()
{
    Condition *Cond;
    llvm::SmallVector<Expr *> Assigns;

    if (expect(Token::KW_loopc))
        goto _error;
    advance();

    Cond = parseConditions();
    if (expect(Token::colon))
        goto _error;
    advance();

    if (expect(Token::KW_begin))
        goto _error;
    advance();

    while (!Tok.is(Token::KW_end))
    {
        Expr *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    return new LOOPC(Cond, Assigns);

}
