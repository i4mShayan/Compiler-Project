#include "Parser.h"

// main point is that the whole input has been consumed
AST *Parser::parse()
{
    AST *Res = parseGSM();
    return Res;
}

AST *Parser::parseGSM()
{
    llvm::SmallVector<Declaration *> Decs;
    llvm::SmallVector<Assignment *> Assigns;
    llvm::SmallVector<IF *> Ifs;
    llvm::SmallVector<LOOP *> Loops;

    while (!Tok.is(Token::eoi))
    {
        switch (Tok.getKind())
        {
        case Token::KW_int:
        {
            Declaration *d;
            d = parseDecl();
            if (d)
                Decs.push_back(d);
            else
                goto _error2;
            break;
        }
        case Token::ident: 
        {
            Assignment *a;
            a = parseAssign();
            if (!Tok.is(Token::semicolon)) {
                error();
                goto _error2;
            } if (a)
                Assigns.push_back(a);
            else goto _error2;
            break;
        } 
        case Token::KW_if:
        {
            IF *i;
            i = parseIf();
            if (i)
                Ifs.push_back(i);
            else
                goto _error2;
            break;
        }
        case Token::KW_loopc:
        {
            Loop *a;
            a = parseLoopc();
            if (a)
                Loops.push_back(a);
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
    return new GSM(Decs, Assigns, Ifs, Loops);
_error2:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Declaration *Parser::parseDec()
{
    int varsCount = 0;
    int expressionCount = 0;
    llvm::SmallVector<Expr *> Exprs;
    llvm::SmallVector<llvm::StringRef, 8> Vars;

    if (expect(Token::KW_int))
        goto _error;

    advance();

    if (expect(Token::ident))
        goto _error;
    Vars.push_back(Tok.getText());
    advance();
    varsCount = 1;

    while (Tok.is(Token::comma))
    {
        advance();
        if (expect(Token::ident))
            goto _error;
        Vars.push_back(Tok.getText());
        varsCount++;
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
            {
                Exprs.push_back(E);
                if (++expressionCount > varsCount)
                {
                    goto _error2
                }
            }
            else
                goto _error;
        }
    }

    if (expect(Token::semicolon))
        goto _error;

    return new Declaration(Vars, Exprs);
_error2:
     // handle the error of vars less than expresions!
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Assignment *Parser::parseAssign()
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
            Expr *L = new BinaryOp(BinaryOp::Plus, F, E);
            return new Assignment(F, L);
        else
            goto _error;
        break;
    }
    case Token::minus_equal:
    {
        advance();
        E = parseExpr();
        if (E)
            Expr *L = new BinaryOp(BinaryOp::Minus, F, E);
            return new Assignment(F, L);
        else
            goto _error;
        break;
    }
    case Token::star_equal:
    {
        advance();
        E = parseExpr();
        if (E)
            Expr *L = new BinaryOp(BinaryOp::Mul, F, E);
            return new Assignment(F, L);
        else
            goto _error;
        break;
    }
    case Token::slash_equal:
    {
        advance();
        E = parseExpr();
        if (E)
            Expr *L = new BinaryOp(BinaryOp::Div, F, E);
            return new Assignment(F, L);
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

Condition *Parser::parseConditions()
{
    return parseCondition();
    // Condition *Left = parseCondition();
    // while (Tok.isOneOf(Token::and, Token:: or))
    // {
    //     LogicalOp::Operator Op =
    //         Tok.is(Token::and) ? LogicalOp::And : LogicalOp::Or;
    //     advance();
    //     Condition *Right = parseCondition();
    //     Left = new LogicalOp(Op, Left, Right);
    // }
    // return Left;
}

Condition *Parser::parseCondition()
{
    Expr *Left = parseExpr();
    ComparisonOp::ComparisonOperator Op;
    switch (Tok.getKind())
    {
    case Token::equal_equal:
        Op = ComparisonOp::eq;
        break;
    case Token::nq:
        Op = ComparisonOp::nq;
        break;
    case Token::ls:
        Op = BinaryOp::ls;
        break;
    case Token::gr:
        Op = ComparisonOp::gr;
        break;
    case Token::le:
        Op = ComparisonOp::le;
        break;
    case Token::ge:
        Op = ComparisonOp::ge;
        break;
    default:
        error();
        return nullptr;
    }
    advance();
    Expr *Right = parseExpr();
    return new ComparisonOp(Op, Left, Right);
}

// IF → "if" CONDITIONS ":" "begin" (ASSIGN)* "end" (ELIF)* (ELSE)?
IF *Parser::parseIf()
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
        Assignment *a;
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
        Assignment *a;
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
        Assignment *a;
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
Loop *Parser::parseLoopc()
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
