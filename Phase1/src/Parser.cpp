#include "Parser.h"

// main point is that the whole input has been consumed
AST *Parser::parse()
{
    AST *Res = parseARK();
    return Res;
}

AST *Parser::parseARK()
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
                if (a)
                    statements.push_back(a);
                else
                    goto _error2;
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
                llvm::errs() << "Here: " << Tok.getText() << "\n";
                goto _error2;
                break;
        }
    }
    return new ARK(statements);
_error2:
    llvm::errs() << "ARK Error at: " << Tok.getText() << "\n";
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
                    goto _error;
                }
            }
            else
                goto _error;
        }
    }

    if (expect(Token::semicolon))
        goto _error;
    advance();

    return new Declare(Vars, Exprs);
_error:
    llvm::errs() << "Declration Error at: " << Tok.getText() << "\n";
    while (Tok.getKind() != Token::eoi)
        
        advance();
    return nullptr;
}

Assign *Parser::parseAssign()
{
    Final *Left;
    Expr *Right;
    Assign *Ans;
    Token::TokenKind tokKind;

    if (expect(Token::ident))
        goto _error;
    Left = (Final*) parseFinal();


    if (!Tok.isOneOf(Token::equal, Token::plus_equal, Token::minus_equal,
    Token::star_equal, Token::slash_equal, Token::mod_equal))
        goto _error;

    tokKind = Tok.getKind();
    advance();

    Right = parseExpr();

    if(!Right) goto _error;

    switch (tokKind)
    {
    case Token::equal:
        Ans = new Assign(Left, Assign::AssOp::EqualAssign, Right);
        break;
    case Token::plus_equal:
        Ans = new Assign(Left, Assign::AssOp::PlusAssign, Right);
        break;
    case Token::minus_equal:
        Ans = new Assign(Left, Assign::AssOp::MinusAssign, Right);
        break;
    case Token::star_equal:
        Ans = new Assign(Left, Assign::AssOp::MulAssign, Right);
        break;
    case Token::slash_equal:
        Ans = new Assign(Left, Assign::AssOp::DivAssign, Right);
        break;
    case Token::mod_equal:
        Ans = new Assign(Left, Assign::AssOp::ModAssign, Right);
        break;
    default:
        goto _error;
        break;
    }

    if (expect(Token::semicolon)) {
        goto _error;
    }
    advance();

    return Ans;
_error:
    llvm::errs() << "Assignment Error at: " << Tok.getText() << "\n";   
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseExpr()
{
    Expr *Left;
    Expr *Right;
    Token::TokenKind tokKind;

    Left = parseTerm();
    while (Tok.isOneOf(Token::plus, Token::minus))
    {
        Expr::Operator Op = Tok.is(Token::plus)
                                    ? Expr::Plus
                                    : Expr::Minus;
        advance();
        Expr *Right = parseTerm();
        Left = new Expr(Left, Op,Right);
    }
    return Left;
    
_error:
    llvm::errs() << "Expression Error at: " << Tok.getText() << "\n";
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;

}

Expr *Parser::parseTerm()
{
    Expr *Left = parseFactor();
    while (Tok.isOneOf(Token::star, Token::slash))
    {
        Expr::Operator Op = Tok.is(Token::star) ? Expr::Mul : Expr::Div;
        advance();
        Expr *Right = parseFactor();
        Left = new Expr(Left, Op,Right);
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
        Left = new Expr(Left, Expr::Pow, Right);
    }
  return Left;
}


Expr *Parser::parseFinal() 
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
        llvm::errs() << "Expected ID/Number/Parentheses but got: " << Tok.getText() << "\n";
        goto _error;
        break;
    }
    return Res;
_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Conditions *Parser::parseConditions()
{
    Condition *Left = nullptr;
    Conditions *Right = nullptr;
    Token::TokenKind tokKind;

    Left = parseCondition();

    if(!Left) goto _error;

    if (!Tok.isOneOf(Token::KW_and, Token::KW_or))
    {
        return (Conditions*) Left;
    }

    tokKind = Tok.getKind();
    advance();

    Right = parseConditions();

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
_error:
    llvm::errs() << "ConditionS Error at: " << Tok.getText() << "\n";
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Condition *Parser::parseCondition()
{
    Expr *Left = nullptr;
    Expr *Right = nullptr;

    Left = parseExpr();

    if(!Left)
        goto _error;

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
        goto _error;
        return nullptr;
    }

    advance();

    Right = parseExpr();

    if(!Right)
        goto _error;

    return new Condition(Left, Op, Right);
_error:
    llvm::errs() << "Condition Error at: " << Tok.getText() << "\n";
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

If *Parser::parseIf()
{
    Conditions *Cond;
    llvm::SmallVector<Assign *> Assigns;
    llvm::SmallVector<Elif *> Elifs;
    Else *ElseBranch = nullptr;

    if (expect(Token::KW_if))
        goto _error;
    advance();

    Cond = parseConditions();
    if(!Cond)
        goto _error;

    if (expect(Token::colon))
        goto _error;
    advance();

    if (expect(Token::KW_begin))
        goto _error;
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
        Elif *Elif;
        Elif = parseElif();
        if (Elif)
            Elifs.push_back(Elif);
        else
            goto _error;
    }

    if (Tok.is(Token::KW_else))
    {
        ElseBranch = parseElse();
        if(!ElseBranch)
            goto _error;
    }

    return new If(Cond, Assigns, Elifs, ElseBranch);

_error:
    llvm::errs() << "If Error at: " << Tok.getText() << "\n";
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Elif *Parser::parseElif()
{
    Conditions *Cond;
    llvm::SmallVector<Assign *> Assigns;

    if (expect(Token::KW_elif))
        goto _error;
    advance();

    Cond = parseConditions();
    if(!Cond)
        goto _error;

    if (expect(Token::colon))
        goto _error;
    advance();

    if (expect(Token::KW_begin))
        goto _error;
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
_error:
    llvm::errs() << "Elif Error at: " << Tok.getText() << "\n";
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Else *Parser::parseElse()
{
    llvm::SmallVector<Assign *> Assigns;

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
        Assign *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    return new Else(Assigns);
_error:
    llvm::errs() << "Else Error at: " << Tok.getText() << "\n";
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Loop *Parser::parseLoop()
{
    Conditions *Cond;
    llvm::SmallVector<Assign *> Assigns;

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
        Assign *a;
        a = parseAssign();
        if (a)
            Assigns.push_back(a);
        else
            goto _error;
    }
    advance();

    return new Loop(Cond, Assigns);

_error:
    llvm::errs() << "Loopc Error at: " << Tok.getText() << "\n";
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}
