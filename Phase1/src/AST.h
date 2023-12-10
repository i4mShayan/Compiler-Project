#ifndef AST_H
#define AST_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

// Forward declarations of classes used in the AST
class AST;
class GSM;
class Assignment;
class Declaration;
class IF;
class ELIF;
class ELSE;
class LOOP;
// class Expr;
// class Factor;
class Final;
// class ID;
// class NUM;
class BinaryOp;
// class ConditionS;
class Condition;

// ASTVisitor class defines a visitor pattern to traverse the AST
class ASTVisitor
{
public:
  // Virtual visit functions for each AST node type
  virtual void visit(AST &) {} // Visit the base AST node
  virtual void visit(Expr &) {}          // Visit the expression node
  virtual void visit(GSM &) = 0;         // Visit the group of expressions node
  virtual void visit(Final &) = 0;       // Visit the final node
  virtual void visit(BinaryOp &) = 0;    // Visit the binary operation node
  virtual void visit(Assignment &) = 0;  // Visit the assignment expression node
  virtual void visit(Declaration &) = 0; // Visit the variable declaration node
  virtual void visit(IF &) = 0;          // Visit the if statement node
  virtual void visit(Condition &) = 0;   // Visit the condtion node
  virtual void visit(ELIF &) = 0;        // Visit the elif statement node
  virtual void visit(ELSE &) = 0;        // Visit the else statement node
  virtual void visit(LOOP &) = 0;        // Visit the loop statement node
};

// AST class serves as the base class for all AST nodes
class AST
{
public:
  virtual ~AST() {}
  virtual void accept(ASTVisitor &V) = 0; // Accept a visitor for traversal
};

// repesent a node in tree
class MyNode : public AST
{
public:
  MyNode() {}
}


// represents an expression in the AST class Expr
class Expr
    : public AST
{
public:
  Expr() {}
};


class Condition
    : public AST
{
public:
  Condition() {}
};

// GSM class represents a group of expressions in the AST
class GSM : public AST
{
  using DeclarationVector = llvm::SmallVector<Declaration *>;
  using AssignVector = llvm::SmallVector<Assignment *>;
  using IfVector = llvm::SmallVector<IF *>;
  using LoopVector = llvm::SmallVector<LOOP *>;

private:
  DeclarationVector decs; // Stores the list of declaration
  AssignVector assigns;   // Stores the list of assignments
  IfVector ifs;           // Stores the list of if statements
  LoopVector loops;       // Stores the list of loop statements

public:
  GSM(llvm::SmallVector<Declaration *> decs, llvm::SmallVector<Assignment *> assigns, llvm::SmallVector<IF *> ifs, llvm::SmallVector<LOOP *> loops)
      : decs(decs), assigns(assigns), ifs(ifs), loops(loops) {}

  DeclarationVector getDeclarations() { return decs; }
  AssignVector getAssignments() { return assigns; }
  IfVector getIfStatements() { return ifs; }
  LoopVector getLoopStatements() { return loops; }

  DeclarationVector::const_iterator beginDeclarations() { return decs.begin(); }
  DeclarationVector::const_iterator endDeclarations() { return decs.end(); }
  AssignVector::const_iterator beginAssignments() { return assigns.begin(); }
  AssignVector::const_iterator endAssignments() { return assigns.end(); }
  IfVector::const_iterator beginIfStatements() { return ifs.begin(); }
  IfVector::const_iterator endIfStatements() { return ifs.end(); }
  LoopVector::const_iterator beginLoopStatements() { return loops.begin(); }
  LoopVector::const_iterator endLoopStatements() { return loops.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Final class represents a Final in the AST (either an identifier or a number)
class Final : public Expr
{
public:
  enum ValueKind
  {
    Ident,
    Number
  };

private:
  ValueKind Kind;      // Stores the kind of final (identifier or number)
  llvm::StringRef Val; // Stores the value of the final

public:
  Final(ValueKind Kind, llvm::StringRef Val) : Kind(Kind), Val(Val) {}

  ValueKind getKind() { return Kind; }

  llvm::StringRef getVal() { return Val; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// BinaryOp class represents a binary operation in the AST (plus, minus, multiplication, division, power)
class BinaryOp : public Expr
{
public:
  enum Operator
  {
    Plus,
    Minus,
    Mul,
    Div,
    Power
  };

private:
  Expr *Left;  // Left-hand side expression
  Expr *Right; // Right-hand side expression
  Operator Op; // Operator of the binary operation

public:
  BinaryOp(Operator Op, Expr *L, Expr *R) : Op(Op), Left(L), Right(R) {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Assignment class represents an assignment expression in the AST
class Assignment : public MyNode
{
private:
  Final *Left; // Left-hand side final (identifier)
  Expr *Right; // Right-hand side expression

public:
  Assignment(Final *L, Expr *R) : Left(L), Right(R) {}

  Final *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Declaration class represents a variable declaration with an initializer in the AST
class Declaration : public MyNode
{
  using VarVector = llvm::SmallVector<llvm::StringRef, 8>;
  VarVector Vars; // Stores the list of variables
  Expr *E;        // Expression serving as the initializer

public:
  Declaration(llvm::SmallVector<llvm::StringRef, 8> Vars, Expr *E) : Vars(Vars), E(E) {}

  VarVector::const_iterator begin() { return Vars.begin(); }

  VarVector::const_iterator end() { return Vars.end(); }

  Expr *getExpr() { return E; }

  virtual void accept(ASTVisitor &V) // override
  {
    V.visit(*this);
  }
};

// IF class represents an if statement in the AST
class IF : public MyNode // TODO: Expr?
{
private:
  Condition *condition;                    // Condition for the if statement
  llvm::SmallVector<Assignment *> assigns; // List of expressions inside the if statement
  llvm::SmallVector<ELIF *> Elifs;         // Optional elif branches
  ELSE *Else;                              // Optional else branch

public:
  IF(Condition *cond, llvm::SmallVector<Assignment *> assign, llvm::SmallVector<ELIF *> e, ELSE *eB) : condition(cond), assigns(assign), Elifs(e), Else(eB) {}

  Condition *getCondition() { return condition; }

  llvm::SmallVector<Assignment *> getAssigns() { return assigns; }

  llvm::SmallVector<ELIF *> getElifs() { return Elifs; }

  ELSE *getElse() { return Else; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// ELIF class represents an elif statement in the AST
class ELIF : public MyNode
{
private:
  Condition *condition;                    // Condition for the elif statement
  llvm::SmallVector<Assignment *> assigns; // List of expressions inside the elif statement

public:
  ELIF(Condition *cond, llvm::SmallVector<Assignment *> assign) : condition(cond), assigns(assign) {}

  Condition *getCondition() { return condition; }

  llvm::SmallVector<Assignment *> getAssigns() { return assigns; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// ELSE class represents an else statement in the AST
class ELSE : public MyNode
{
private:
  llvm::SmallVector<Assignment *> assigns; // List of expressions inside the else statement

public:
  ELSE(llvm::SmallVector<Assignment *> assign) : assigns(assign) {}

  llvm::SmallVector<Assignment *> getAssigns() { return assigns; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// LOOP class represents a loop statement in the AST
class LOOP : public MyNode
{
private:
  Condition *condition;                    // Condition for the loop statement
  llvm::SmallVector<Assignment *> assigns; // List of expressions inside the loop body

public:
  LOOP(Condition *cond, llvm::SmallVector<Assignment *> assign) : condition(cond), assigns(assign) {}

  Condition *getCondition() { return condition; }

  llvm::SmallVector<Assignment *> getAssigns() { return assigns; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class LogicalOp : public MyNode
{
public:
  enum LogicOperator
  {
    Or,
    And
  };

private:
  Expr *Left;         // Left-hand side expression
  Expr *Right;        // Right-hand side expression
  LogicOperator Op; // Operator of the binary operation

public:
  LogicalOp(LogicOperator Op, Expr *L, Expr *R) : LogicOperator(Op), Left(L), Right(R) {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  LogicalOperator getLogicalOperator() { return Op; }

  virtual void accept(ASTVisitor &V) //override
  {
    V.visit(*this);
  }
};

// class AssignOp : public MyNode
// {
// public:
//   enum AssignOperator
//   {
//     Minus_equal,
//     Plus_equal,
//     Star_equal,
//     Slash_equal,
//     Equal,
//   };

// private:
//   Final *Left;         
//   Expr *Right;        // Right-hand side expression
//   AssignOperator AssignOp; // Operator of the binary operation

// public:
//   LogicalOp(Final *L, Expr *R, AssignOperator Op) : Left(L), Right(R), AssignOp(Op) {}

//   Final *getLeft() { return Left; }

//   Expr *getRight() { return Right; }

//   LogicalOperator getAssignOperator() { return Op; }

//   virtual void accept(ASTVisitor &V) //override
//   {
//     V.visit(*this);
//   }
// };

class ComparisonOp : public Condition
{
public:
  enum ComparisonOperator
  {
    ls,
    le,
    eq,
    gr,
    ge,
    nq
  };

private:
  Expr *Left;            // Left-hand side expression
  Expr *Right;           // Right-hand side expression
  ComparisonOperator operator; // Operator of the binary operation

public:
  ComparisonOp(ComparisonOperator Op, Expr *L, Expr *R) : operator(Op), Left(L), Right(R) {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  ComparisonOperator getComparisonOperatorr() { return operator; }

  virtual void accept(ASTVisitor &V) //override
  {
    V.visit(*this);
  }
};

#endif