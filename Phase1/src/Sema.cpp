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

    // Visit function for ARK nodes
    virtual void visit(ARK &Node) override
    {
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this); // Visit each child node
      }
    };

    // Visit function for final nodes
    virtual void visit(Final &Node) override
    {
      if (Node.getKind() == Final::Ident)
      {
        // Check if identifier is in the scope
        if (Scope.find(Node.getVal()) == Scope.end())
          error(Not, Node.getVal());
      }
    };

    // Visit function for Expr nodes
    virtual void visit(Expr &Node) override
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

      if (Node.getOperator() == Expr::Operator::Div && right)
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
    virtual void visit(Assign &Node) override
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
        Node.getRight()->accept(*this); 

      if (Node.getAssignmentOP() == Assign::AssOp::Div && Node.getRight())
      {
        Final *f = (Final *)Node.getRight();

        if (Node.getRight() && f->getKind() == Final::ValueKind::Number)
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

    virtual void visit(Declare &Node) override
    {
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        if (!Scope.insert(*I).second)
          error(Twice, *I); // If the insertion fails (element already exists in Scope), report a "Twice" error
      }
      if (Node.getExprs())
        for (auto I = Node.getExprs()->begin(), E = Node.getExprs()->end(); I != E; ++I)
          (*I)->accept(*this);
    };
  

  virtual void visit(If &Node) override
  {
    for (auto I = Node.getConds()->begin(), E = Node.getConds()->end(); I != E; ++I)
      (*I)->accept(*this);

    if (Node.getAssignments())
      for (auto I = Node.getAssignments()->begin(), E = Node.getAssignments()->end(); I != E; ++I)
        (*I)->accept(*this);

    if (Node.getElifs())
      for (auto I = Node.getElifs()->begin(), E = Node.getElifs()->end(); I != E; ++I)
        (*I)->accept(*this);
    
    if (Node.getElseBranch())
      Node.getElseBranch()->accept(*this);
  };

  virtual void visit(Elif &Node) override
  {
    if (Node.getConds())
      for (auto I = Node.getConds()->begin(), E = Node.getConds()->end(); I != E; ++I)
        (*I)->accept(*this);

    if (Node.getAssignments())
      for (auto I = Node.getAssignments()->begin(), E = Node.getAssignments()->end(); I != E; ++I)
        (*I)->accept(*this);
  };

  virtual void visit(Else &Node) override
  {
    if (Node.getAssignments())
      for (auto I = Node.getAssignments()->begin(), E = Node.getAssignments()->end(); I != E; ++I)
        (*I)->accept(*this);
  };

  virtual void visit(Loop &Node) override
  {
    if (Node.getConds())
      for (auto I = Node.getConds()->begin(), E = Node.getConds()->end(); I != E; ++I)
        (*I)->accept(*this);

    if (Node.getAssignments())
      for (auto I = Node.getAssignments()->begin(), E = Node.getAssignments()->end(); I != E; ++I)
        (*I)->accept(*this);
  };
  
  virtual void visit(Condition &Node) override
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

  };
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
