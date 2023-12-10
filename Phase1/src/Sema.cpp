#include "Sema.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

namespace
{
  class InputCheck : public ASTVisitor
  {
    llvm::StringSet<> Scope; // StringSet to store declared variables
    bool HasError;           // Flag to indicate if an error occurred

    enum ErrorType
    {
      Twice,
      Not
    }; // Enum to represent error types: Twice - variable declared twice, Not - variable not declared

    void error(ErrorType ET, llvm::StringRef V)
    {
      // Function to report errors
      llvm::errs() << "Variable " << V << " is "
                   << (ET == Twice ? "already" : "not")
                   << " declared\n";
      HasError = true; // Set error flag to true
    }

  public:
    InputCheck() : HasError(false) {} // Constructor

    bool hasError() { return HasError; } // Function to check if an error occurred

    // Visit function for GSM nodes
    virtual void visit(GSM &Node) override
    {
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this); // Visit each child node
      }
    };

    // Visit function for final nodes
    virtual void visit(ّFinal &Node) override
    {
      if (Node.getKind() == Final::Ident)
      {
        // Check if identifier is in the scope
        if (Scope.find(Node.getVal()) == Scope.end())
          error(Not, Node.getVal());
      }
    };

    // Visit function for BinaryOp nodes
    virtual void visit(BinaryOp &Node) override
    {
      if (Node.getLeft())
        Node.getLeft()->accept(*this);
      else
        HasError = true;

      auto right = Node.getRight();
      if (right)
        right->accept(*this);
      else
        HasError = true;

      if (Node.getOperator() == BinaryOp::Operator::Div && right)
      {
        Final *f = (Final *)right;

        if (right && f->getKind() == Final::ValueKind::Number)
        {
          int intval;
          f->getVal().getAsInteger(10, intval);

          if (intval == 0)
          {
            llvm::errs() << "Division by zero is not allowed."
                         << "\n";
            HasError = true;
          }
        }
      }
    };

    // Visit function for Assignment nodes
    virtual void visit(Assignment &Node) override
    {
      Final *dest = Node.getLeft();

      dest->accept(*this);

      if (dest->getKind() == Final::Number)
      {
        llvm::errs() << "Assignment destination must be an identifier.";
        HasError = true;
      }

      if (dest->getKind() == Final::Ident)
      {
        // Check if the identifier is in the scope
        if (Scope.find(dest->getVal()) == Scope.end())
          error(Not, dest->getVal());
      }

      if (Node.getRight())
        Node.getRight()->accept(*this); /// How this works ? the right node is EXPR wich can be parsed to binaryOP or a Final node
    };

    virtual void visit(Declaration &Node) override
    {
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        if (!Scope.insert(*I).second)
          error(Twice, *I); // If the insertion fails (element already exists in Scope), report a "Twice" error
      }
      if (Node.getExpr())
        Node.getExpr()->accept(*this); // If the Declaration node has an expression, recursively visit the expression node
    };
  };
  virtual void visit(IF &Node) override
  {
    if (Node.getCondition())
      Node.getCondition()->accept(*this);
    else
      HasError = true;

    if (Node.getAssigns())
      for (auto I = Node.getAssigns()->begin(), E = Node.getAssigns()->end(); I != E; ++I)
        (*I)->accept(*this);
    else
      HasError = true;

    if (Node.getElif())
      for (auto I = Node.getElif()->begin(), E = Node.getElif()->end(); I != E; ++I)
        (*I)->accept(*this);
    else
      HasError = true;
    if (Node.getElse())
      Node.getElse()->accept(*this);
    else
      HasError = true;
  };
  virtual void visit(ELIF &Node) override
  {
    if (Node.getCondition())
      Node.getCondition()->accept(*this);
    else
      HasError = true;

    if (Node.getAssigns())
      for (auto I = Node.getAssigns()->begin(), E = Node.getAssigns()->end(); I != E; ++I)
        (*I)->accept(*this);
    else
      HasError = true;
  };
  virtual void visit(ELSE &Node) override
  {
    if (Node.getAssigns())
      for (auto I = Node.getAssigns()->begin(), E = Node.getAssigns()->end(); I != E; ++I)
        (*I)->accept(*this);
    else
      HasError = true;
  };
  virtual void visit(LoopC &Node) override
  {
    if (Node.getCondition())
      Node.getCondition()->accept(*this);
    else
      HasError = true;

    if (Node.getAssigns())
      for (auto I = Node.getAssigns()->begin(), E = Node.getAssigns()->end(); I != E; ++I)
        (*I)->accept(*this);
    else
      HasError = true;
  };
  virtual void visit(Condition &Node) override
  {
    if (Node.getLeft())
      Node.getLeft()->accept(*this);
    else
      HasError = true;

    if (Node.getRight())
      Node.getRight()->accept(*this);
    else
      HasError = true;
  };
}

bool Sema::semantic(AST *Tree)
{
  if (!Tree)
    return false; // If the input AST is not valid, return false indicating no errors

  InputCheck Check;    // Create an instance of the InputCheck class for semantic analysis
  Tree->accept(Check); // Initiate the semantic analysis by traversing the AST using the accept function

  return Check.hasError(); // Return the result of Check.hasError() indicating if any errors were detected during the analysis
}
