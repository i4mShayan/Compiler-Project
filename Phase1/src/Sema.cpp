#include "Sema.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class InputCheck : public ASTVisitor {
  llvm::StringSet<> Scope; // StringSet to store declared variables
  bool HasError; // Flag to indicate if an error occurred

  enum ErrorType { Twice, Not }; // Enum to represent error types: Twice - variable declared twice, Not - variable not declared

  void error(ErrorType ET, llvm::StringRef V) {
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
  virtual void visit(ARK &Node) override {
    for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
    {
      (*I)->accept(*this); // Visit each child node
    }
  };


  virtual void visit(Statement &Node) override {
    switch (Node.getKind())
    {
      case Statement::Declaration:
      {
        Declare *dec = dynamic_cast<Declare*> (Node*);
        dec->accept(*this);
        break;
      }
      case Statement::Assignment:
      {
        Assign *assign = dynamic_cast<Assign*> (Node*);
        assign->accept(*this);
        break;
      }
      case Statement::If:
      {
        If *if_condition = dynamic_cast<If*> (Node*);
        if_condition->accept(*this);
        break;
      }
      case Statement::Loop:
      {
        Loop *loop = dynamic_cast<Loop*> (Node*);
        loop->accept(*this);
        break;
      }
      default:
        break;
    }
  };


  virtual void visit(Declare &Node) override {
    for (auto I = Node.VarsBegin(), E = Node.VarsEnd(); I != E; ++I) {
      if (!Scope.insert(*I).second)
        error(Twice, *I); // If the insertion fails (element already exists in Scope), report a "Twice" error
    }
    if (Node.getExprs()) {
      for (auto I = Node.ExprsBegin(), E = Node.ExprsEnd(); I != E; ++I) {
         (*I)->accept(*this);
      }
    }
  };


  virtual void visit(Assign &Node) override {
    Final *left = Node.getLeft();

    left->accept(*this);

    if (Scope.find(left->getVal()) == Scope.end())
      error(Not, left->getVal()); // Variable Not Found

    Node.getRight()->accept(*this);
  };


  virtual void visit(Expr &Node) override {
    Node.getLeft()->accept(*this);

    auto right = Node.getRight();
    if (Node.getRight())
      right->accept(*this);

    if (Node.getOperator() == Expr::Div && right) {
      Final * f = (Final *)right;

      if (right && f->getKind() == Final::Number) {
        int intval;
        f->getVal().getAsInteger(10, intval);

        if (intval == 0) {
          llvm::errs() << "Division by zero is not allowed." << "\n";
          HasError = true;
        }
      }
    }
  };


  virtual void visit(Conditions &Node) override {
    Node.getLeft()->accept(*this);
    if(Node.getRight()) Node.getRight()->accept(*this);
  };


  virtual void visit(Condition &Node) override {
    Node.getLeft()->accept(*this);
    Node.getRight()->accept(*this);
  };


  virtual void visit(Final &Node) override {
    if (Node.getKind() == Final::Ident) {
      // Check if identifier is in the scope
      if (Scope.find(Node.getVal()) == Scope.end())
        error(Not, Node.getVal()); // Variable Not Found
    }
  };


  virtual void visit(If &Node) override {
    Node.getConds()->accept(*this);

    for (auto I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }

    for (auto I = Node.ElifsBegin(), E = Node.ElifsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }

    if(Node.getElse()) Node.getElse()->accept(*this);
  };


  virtual void visit(Elif &Node) override {
    Node.getConds()->accept(*this);

    for (auto I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }
  };


  virtual void visit(Else &Node) override {
    for (auto I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }
  };


  virtual void visit(Loop &Node) override {
    Node.getConds()->accept(*this);

    for (auto I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }
  };
};
}

bool Sema::semantic(AST *Tree) {
  if (!Tree)
    return false; // If the input AST is not valid, return false indicating no errors

  InputCheck Check; // Create an instance of the InputCheck class for semantic analysis
  Tree->accept(Check); // Initiate the semantic analysis by traversing the AST using the accept function

  return Check.hasError(); // Return the result of Check.hasError() indicating if any errors were detected during the analysis
}