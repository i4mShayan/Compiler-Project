#include "Parser.h"

// main point is that the whole input has been consumed
AST *Parser::parse()
{
    AST *Res = parseGSM();
    return Res;
}

AST *Parser::parseGSM()
{
    llvm::SmallVector<Statement *> statements;

    while (!Tok.is(Token::eoi))
    {
        switch (Tok.getKind())
        {
        case Token::KW_int:
        {
            Declare *d;
            d = parseDec();
            if (d)
                statements.push_back(d);
            else
                goto _error2;
            break;
        }
        case Token::ident: 
        {
            Assign *a;
            a = parseAssign();
            if (!Tok.is(Token::semicolon)) {
                error();
                goto _error2;
            } if (a)
                statements.push_back(a);
            else goto _error2;
            break;
        } 
        case Token::KW_if:
        {
            If *i;
            i = parseIf();
            if (i)
                statements.push_back(i);
            else
                goto _error2;
            break;
        }
        case Token::KW_loopc:
        {
            Loop *a;
            a = parseLoop();
            if (a)
                statements.push_back(a);
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
    return new GSM(statements);
_error2:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Declare *Parser::parseDec()
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
                    goto _error2;
                }
            }
            else
                goto _error;
        }
    }

    if (expect(Token::semicolon))
        goto _error;

    return new Declare(Vars, Exprs);
_error2:
     // handle the error of vars less than expresions!
_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Assign *Parser::parseAssign()
{
    Final *F;
    Expr *E;
    F = (Final *)(parseFinal());


    Token::TokenKind tokKind = Tok.getKind();

    advance();
    E = parseExpr();

    if(!E) goto _error;

    switch (Tok.getKind())
    {
    case Token::equal:
        return new Assign(F, Assign::AssOp::EqualAssign, E);
    case Token::plus_equal:
        return new Assign(F, Assign::AssOp::PlusAssign, E);
    case Token::minus_equal:
        return new Assign(F, Assign::AssOp::MinusAssign, E);
    case Token::star_equal:
        return new Assign(F, Assign::AssOp::MulAssign, E);
    case Token::slash_equal:
        return new Assign(F, Assign::AssOp::MulAssign, E);
    default:
        goto _error;
        break;
    }

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseExpr()
{
    Final *Left;
    Expr *Right;

    Left = (Final *) parseFinal();

    if (! Tok.isOneOf(Token::plus, Token::minus, Token::star,
    Token::slash, Token::mod, Token::hat))
    {
        return new Expr(Final);
    }

    Token::TokenKind tokKind = Tok.getKind();

    advance();
    Right = parseExpr();

    if (!Right) goto _error;

    switch (tokKind)
    {
        case Token::plus:
            return new Expr(Left, Expr::Operator::Plus, Right);
        case Token::minus:
            return new Expr(Left, Expr::Operator::Minus, Right);
        case Token::star:
            return new Expr(Left, Expr::Operator::Mul, Right);
        case Token::slash:
            return new Expr(Left, Expr::Operator::Div, Right);
        case Token::mod:
            return new Expr(Left, Expr::Operator::Mod, Right);
        case Token::hat:
            return new Expr(Left, Expr::Operator::Pow, Right);
        default:
        {
            goto _error;
            break;
        }
    }

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}


Expr *Parser::parseFinal() // the return type MUST be Expr
{
    Expr *Res;
    switch (Tok.getKind())
    {
    case Token::number:
        Res = new Final(Final::ValueKind::Number, Tok.getText());
        advance();
        break;
    case Token::ident:
        Res = new Final(Final::ValueKind::Ident, Tok.getText());
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

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Conditions *Parser::parseConditions()
{
    Condition *Left;
    Conditions *Right;

    Left = parseCondition();

    if (!Tok.isOneOf(Token::KW_and, Token::KW_or))
    {
        return new Conditions(Left);
    }

    Token::TokenKind tokKind = Tok.getKind();

    advance();
    Right = parseCondition();

    if (!Right) goto _error;

    switch (tokKind)
    {
        case Token::KW_and:
            return new Conditions(Left, Conditions::Operator::And, Right);
        case Token::KW_or:
            return new Conditions(Left, Conditions::Operator::Or, Right);
        default:
        {
            goto _error;
            break;
        }
    }

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Condition *Parser::parseCondition()
{
    Expr *Left = parseExpr();

    if(!Left) goto _error;

    Condition::Operator Op;
    switch (Tok.getKind())
    {
    case Token::equal_equal:
        Op = Condition::Operator::EqualEqual;
        break;
    case Token::nq:
        Op = Condition::Operator::NotEqual;
        break;
    case Token::ls:
        Op = Condition::Operator::LessThan;
        break;
    case Token::gr:
        Op = Condition::Operator::GreaterThan;
        break;
    case Token::le:
        Op = Condition::Operator::LessEqual;
        break;
    case Token::ge:
        Op = Condition::Operator::GreaterEqual;
        break;
    default:
        error();
        return nullptr;
    }

    advance();
    Expr *Right = parseExpr();

    if(!Right) goto _error;

    return new Condition(Left, Op, Right);

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

// If → "If" CONDITIONS ":" "begin" (ASSIGN)* "end" (ELIf)* (ELSE)?
If *Parser::parseIf()
{
    Condition *Cond;
    llvm::SmallVector<Assign *> Assigns;
    llvm::SmallVector<ELIf *> Elifs;
    Else *Else;

    if (expect(Token::KW_If)) goto _error;
    advance();

    Cond = parseCondition();
    if (expect(Token::colon)) goto _error;
    advance();

    if (expect(Token::KW_begin)) goto _error;
    advance();

    while (!Tok.is(Token::KW_end))
    {
        Assign *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    while (Tok.is(Token::KW_elif))
    {
        // advance(); It is not necessary to advance here because parseElIf() will do it
        Elif *Elif;
        Elif = parseElif();
        if (Elif)
            Elifs.push_back(Elif);
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

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

// ELIf → "elIf" CONDITIONS ":" "begin" (ASSIGN)* "end"
Elif *Parser::parseElif()
{
    Condition *Cond;
    llvm::SmallVector<Assign *> Assigns;

    if (expect(Token::KW_elif)) goto _error;
    advance();

    Cond = parseCondition();
    if (expect(Token::colon)) goto _error;
    advance();

    if (expect(Token::KW_begin)) goto _error;
    advance();

    while (!Tok.is(Token::KW_end))
    {
        Assign *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    return new Elif(Cond, Assigns);

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

// ELSE→ "else" ":" "begin" (ASSIGN)* "end"

Else *Parser::parseElse()
{
    llvm::SmallVector<Assign *> Assigns;

    if (expect(Token::KW_else)) goto _error;
    advance();

    if (expect(Token::colon)) goto _error;
    advance();

    if (expect(Token::KW_begin)) goto _error;
    advance();

    while (!Tok.is(Token::KW_end))
    {
        Assign *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    return new Else(Assigns);

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

// LoopC → "Loopc" CONDITIONS ":" "begin" (ASSIGN)* "end"
Loop *Parser::parseLoop()
{
    Condition *Cond;
    llvm::SmallVector<Assign *> Assigns;

    if (expect(Token::KW_loopc)) goto _error;
    advance();

    Cond = parseCondition();
    if (expect(Token::colon)) goto _error;
    advance();

    if (expect(Token::KW_begin)) goto _error;
    advance();

    while (!Tok.is(Token::KW_end))
    {
        Assign *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    return new Loop(Cond, Assigns);

_error: // TODO: Check this later in case of error :)
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}
