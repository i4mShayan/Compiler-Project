#ifndef AST_H
#define AST_H

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"

class AST; // Abstract Syntax Tree
class ARK; // Top-level program
class Statement; // Top-level statement
class Expr; // Binary operation of numbers and identifiers
class Assign; // Assignment statement like a = 3;
class Declare; // Declaration statement like int a;
class If;
class Elif;
class Else;
class Condition;
class Loop;
class Conditions;
class Final;

class ASTVisitor
{
public:
    // Virtual visit functions for each AST node type
    virtual void visit(AST &) {}
    virtual void visit(Expr &) {}
    virtual void visit(ARK &) = 0;
    virtual void visit(Statement &) = 0;
    virtual void visit(Declare &) = 0;//
    virtual void visit(Assign &) = 0;//
    virtual void visit(If &) = 0;
    virtual void visit(Elif &) = 0;
    virtual void visit(Else &) = 0;
    virtual void visit(Condition &) = 0;
    virtual void visit(Loop &) = 0;
    virtual void visit(Conditions &) = 0;
    virtual void visit(Final &) = 0;
};

class AST
{
public:
    virtual ~AST() {}
    virtual void accept(ASTVisitor &V) = 0;
};

class ARK : public AST
{
private:
    llvm::SmallVector<Statement *> statements; // Stores the list of Exprs

public:
    ARK(llvm::SmallVector<Statement *> Statements) : statements(Statements) {}
    llvm::SmallVector<Statement *> getStatements() { return statements; }

    llvm::SmallVector<Statement *>::const_iterator begin() { return statements.begin(); }

    llvm::SmallVector<Statement *>::const_iterator end() { return statements.end(); }
    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Statement : public AST
{
public:
    enum StatementType
    {
        Declaration,
        Assignment,
        If,
        Loop
    };

private:
    StatementType Type;

public:
    StatementType getKind()
    {
        return Type;
    }


    Statement(StatementType type) : Type(type) {}
    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Declare : public Statement
{
private:
    llvm::SmallVector<llvm::StringRef, 8> vars;
    llvm::SmallVector<Expr *> exprs;

public:
    Declare(llvm::SmallVector<llvm::StringRef, 8> vars, llvm::SmallVector<Expr *> exprs) :
        vars(vars), exprs(exprs), Statement(Statement::StatementType::Declaration) {}

    llvm::SmallVector<llvm::StringRef, 8> getVars()
    {
        return vars;
    }

    llvm::SmallVector<Expr *> getExprs()
    {
        return exprs;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Assign : public Statement
{
public:
    enum AssOp
    {
        PlusAssign,
        MinusAssign,
        MulAssign,
        DivAssign,
        ModAssign,
        EqualAssign
    };

private:
    Final *Left;
    AssOp AssignmentOp;
    Expr *Right;

public:
    Assign(Final *Left, AssOp AssignmentOp, Expr *Right) :
     Left(Left), AssignmentOp(AssignmentOp), Right(Right), Statement(StatementType::Assignment) {}
    Final *getLeft()
    {
        return Left;
    }

    Expr *getRight()
    {
        return Right;
    }

    AssOp getAssignmentOP()
    {
        return AssignmentOp;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Expr : public AST
{
public:
    enum Operator
    {
        Plus,
        Minus,
        Mul,
        Div,
        Mod,
        Pow
    };

private:
    Final *Left; // Left-hand side Expr
    Expr *Right; // Right-hand side Expr
    Operator Op;      // Operator of the binary operation

public:
    Expr(Final *L, Operator Op, Expr *R) : 
    Left(L), Op(Op), Right(R) {}
    Expr(Final *L) : 
    Left(L) {}
    Expr() {}

    Final *getLeft() { return Left; }

    Operator getOperator() { return Op; }

    Expr *getRight() { return Right; }


    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Conditions : public AST
{
public:
    enum Operator
    {
        And,
        Or
    };
private:
    Condition *Left;
    Operator Sign;
    Conditions *Right;

public:
    Conditions(Condition *left, Operator sign, Conditions *right) : 
    Left(left), Sign(sign), Right(right) {}
    Conditions(Condition *left) : 
    Left(left) {}
    Conditions() {}

    Condition *getLeft() { return Left; }

    Operator getSign() { return Sign; }

    Conditions *getRight() { return Right; }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Condition : public Conditions
{
public:
    enum Operator
    {
        LessEqual,
        LessThan,
        GreaterThan,
        GreaterEqual,
        EqualEqual,
        NotEqual
    };

private:
    Expr *Left; // Left-hand side Expr
    Expr *Right; // Right-hand side Expr
    Operator Op;      // Operator of the boolean operation

public:
    Condition(Expr *left, Operator Op, Expr *right) : 
    Left(left), Op(Op), Right(right), Conditions() {}

    Expr *getLeft() { return Left; }

    Operator getSign() { return Op; }

    Expr *getRight() { return Right; }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Final : public Expr
{
public:
  enum ValueKind
  {
    Ident,
    Number
  };

private:
  ValueKind Kind;                            
  llvm::StringRef Val;

public:
  Final(ValueKind Kind, llvm::StringRef Val) : Kind(Kind), Val(Val), Expr() {}

  ValueKind getKind() { return Kind; }

  llvm::StringRef getVal() { return Val; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};


class If : public Statement
{

private:
    Conditions *Conditions;
    llvm::SmallVector<Assign *> Assignments;
    llvm::SmallVector<Elif *> Elifs;
    Else *ElseBranch;

public:
    If(Conditions *Conditions, llvm::SmallVector<Assign *> Assignments,llvm::SmallVector<Elif *> Elifs,Else *ElseBranch) : 
    Conditions(Conditions), Assignments(Assignments), Statement(Statement::StatementType::If), Elifs(Elifs), ElseBranch(ElseBranch) {}
    If(): Statement(Statement::StatementType::If) {}

    Conditions *getConditions()
    {
        return Conditions;
    }

    llvm::SmallVector<Assign *> getAssignments()
    {
        return Assignments;
    }

    llvm::SmallVector<Elif *> getElifs()
    {
        return EliConditionsLofs;
    }

    Else *getElseBranch()
    {
        return ElseBranch;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Elif : public If
{
    
private:
    Conditions *Conditions;
    llvm::SmallVector<Assign *> Assignments;

public:
    Elif(Conditions *Conditions, llvm::SmallVector<Assign *> Assignments) :
     Conditions(Conditions), Assignments(Assignments), If() {}

    Conditions *getConditions()
    {
        return Conditions;
    }

    llvm::SmallVector<Assign *> getStatements()
    {
        return Assignments;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Else : public If
{

private:
    llvm::SmallVector<Assign *> Assignments;

public:
    Else(llvm::SmallVector<Assign *> Assignments) : 
    Assignments(Assignments), If() {}

    llvm::SmallVector<Assign *> getAssignments()
    {
        return Assignments;
    }

virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Loop : public Statement
{

private:
    Conditions *Conditions;
    llvm::SmallVector<Assign *> Assignments;

public:
    Loop(Conditions *Conditions, llvm::SmallVector<Assign *> Assignments) : 
    Conditions(Conditions), Assignments(Assignments), Statement(Statement::StatementType::If) {}

    Conditions *getConditions()
    {
        return Conditions;
    }

    llvm::SmallVector<Assign *> getAssignments()
    {
        return Assignments;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

#endif