#ifndef AST_H
#define AST_H

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"

class AST; // Abstract Syntax Tree
class ARK; // Top-level program
class Statement; // Top-level statement
class Expr; 
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
    virtual void visit(AST &) {};
    virtual void visit(ARK &) {};
    virtual void visit(Statement &) {};
    virtual void visit(Declare &) = 0;
    virtual void visit(Assign &) = 0;
    virtual void visit(Expr &) = 0;
    virtual void visit(If &) {};
    virtual void visit(Elif &) {};
    virtual void visit(Else &) {};
    virtual void visit(Conditions &) {};
    virtual void visit(Condition &) {};
    virtual void visit(Loop &) {};
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

    llvm::SmallVector<Statement *> erase(Statement* statement) {
        llvm::SmallVector<Statement *>::iterator iter = std::find(statements.begin(), statements.end(), statement);

        // Check if the element was found
        if (iter != statements.end()) {
            // Erase the element from the vector
            statements.erase(iter);
        }

        return statements;
    }

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
    StatementType getKind() { return Type; }

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
        vars(vars), exprs(exprs), Statement(Statement::Declaration) {}

    llvm::SmallVector<llvm::StringRef, 8> getVars()
    {
        return vars;
    }

    llvm::SmallVector<Expr *> getExprs()
    {
        return exprs;
    }

    llvm::SmallVector<llvm::StringRef, 8>::const_iterator VarsBegin() { return vars.begin(); }

    llvm::SmallVector<llvm::StringRef, 8>::const_iterator VarsEnd() { return vars.end(); }

    llvm::SmallVector<Expr *>::const_iterator ExprsBegin() { return exprs.begin(); }

    llvm::SmallVector<Expr *>::const_iterator ExprsEnd() { return exprs.end(); }

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

    Final *getLeft() { return Left; }

    Expr *getRight() { return Right; }

    AssOp getAssignmentOP() { return AssignmentOp; }

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
    Expr *Left = nullptr; 
    Operator Op;      
    Expr *Right = nullptr; 

public:
    Expr(Expr *L, Operator Op, Expr *R) : 
    Left(L), Op(Op), Right(R) {}
    Expr(Expr *L) : 
    Left(L) {}
    Expr() {}

    Expr *getLeft() { return Left; }

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
    Conditions *Right = nullptr;

public:
    Conditions(Condition *Left, Operator Sign, Conditions *Right) : 
    Left(Left), Sign(Sign), Right(Right) {}
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
    Operator Op;      // Operator of the boolean operation
    Expr *Right; // Right-hand side Expr

public:
    Condition(Expr *Left, Operator Op, Expr *Right) : 
    Left(Left), Op(Op), Right(Right), Conditions() {}

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
    Conditions *Conds;
    llvm::SmallVector<Assign *> Assignments;
    llvm::SmallVector<Elif *> Elifs;
    Else *ElseBranch;

public:
    If(Conditions *Conds, llvm::SmallVector<Assign *> Assignments,llvm::SmallVector<Elif *> Elifs, Else *ElseBranch) : 
    Conds(Conds), Assignments(Assignments), Statement(Statement::If), Elifs(Elifs), ElseBranch(ElseBranch) {}
    
    If() : Statement(Statement::If) {}

    Conditions *getConds() { return Conds; }

    llvm::SmallVector<Assign *> getAssignments() { return Assignments; }

    llvm::SmallVector<Assign *>::const_iterator AssignmentsBegin() { return Assignments.begin(); }

    llvm::SmallVector<Assign *>::const_iterator AssignmentsEnd() { return Assignments.end(); }

    llvm::SmallVector<Elif *> getElifs() { return Elifs; }

    llvm::SmallVector<Elif *>::const_iterator ElifsBegin() { return Elifs.begin(); }

    llvm::SmallVector<Elif *>::const_iterator ElifsEnd() { return Elifs.end(); }

    Else *getElse() { return ElseBranch; }

    virtual void accept(ASTVisitor &V) override { V.visit(*this); }
};

class Elif : public If
{
private:
    Conditions *Conds;
    llvm::SmallVector<Assign *> Assignments;

public:
    Elif(Conditions *Conds, llvm::SmallVector<Assign *> Assignments) :
     Conds(Conds), Assignments(Assignments), If() {}

    Conditions *getConds() { return Conds; }

    llvm::SmallVector<Assign *> getAssignments() { return Assignments; }

    llvm::SmallVector<Assign *>::const_iterator AssignmentsBegin() { return Assignments.begin(); }

    llvm::SmallVector<Assign *>::const_iterator AssignmentsEnd() { return Assignments.end(); }

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

    llvm::SmallVector<Assign *> getAssignments() { return Assignments; }

    llvm::SmallVector<Assign *>::const_iterator AssignmentsBegin() { return Assignments.begin(); }

    llvm::SmallVector<Assign *>::const_iterator AssignmentsEnd() { return Assignments.end(); }


virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class Loop : public Statement
{
private:
    Conditions *Conds;
    llvm::SmallVector<Assign *> Assignments;

public:
    Loop(Conditions *Conds, llvm::SmallVector<Assign *> Assignments) : 
    Conds(Conds), Assignments(Assignments), Statement(Statement::Loop) {}

    Conditions *getConds() { return Conds; }

    llvm::SmallVector<Assign *> getAssignments() { return Assignments; }

    llvm::SmallVector<Assign *>::const_iterator AssignmentsBegin() { return Assignments.begin(); }

    llvm::SmallVector<Assign *>::const_iterator AssignmentsEnd() { return Assignments.end(); }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

#endif