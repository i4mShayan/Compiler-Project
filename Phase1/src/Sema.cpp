#include "Sema.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

namespace {

class InputCheck : public ASTVisitor {
  llvm::StringSet<> Scope; // StringSet to store declared variables
  bool HasError; // Flag to indicate if an error occurred

  enum ErrorType { Twice, Not }; // Enum to represent error types: Twice - variable declared twice, Not - variable not declared

  void declare_error(ErrorType ET, llvm::StringRef V) {
    llvm::errs() << "Variable " << V << " is "
                 << (ET == Twice ? "already" : "not")
                 << " declared\n";
    HasError = true;
  }

  void divide_by_zero_error() {
    llvm::errs() << "Division/Modulo by zero is not allowed." << "\n";
    HasError = true;
  }


public:
  InputCheck() : HasError(false) {} // Constructor

  bool hasError() { return HasError; } // Function to check if an error occurred

  // Visit function for GSM nodes
  virtual void visit(ARK &Node) override {
    for (llvm::SmallVector<Statement *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
    {
      (*I)->accept(*this); // Visit each child node
    }
  };

  virtual void visit(Statement &Node) override 
  {
    llvm::errs() << "kiram to rafiee. " << "\n";

  };

  virtual void visit(Declare &Node) override {
    for (llvm::SmallVector<llvm::StringRef, 8>::const_iterator I = Node.VarsBegin(), E = Node.VarsEnd(); I != E; ++I) {
      if (!Scope.insert(*I).second)
        declare_error(Twice, *I); // If the insertion fails (element already exists in Scope), report a "Twice" error
    }
    for (llvm::SmallVector<Expr *>::const_iterator I = Node.ExprsBegin(), E = Node.ExprsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }
  };


  virtual void visit(Assign &Node) override {
    Final *left = Node.getLeft();
    Expr *right = Node.getRight();

    left->accept(*this);

    right->accept(*this);

    if (Node.getAssignmentOP() == Assign::DivAssign || Node.getAssignmentOP() == Assign::ModAssign) {
      Final * f = (Final *)right;
      if (f->getKind() == Final::Number) {
        int intval;
        f->getVal().getAsInteger(10, intval);

        if (intval == 0) divide_by_zero_error();
      }
    }
  };


  virtual void visit(Expr &Node) override {
    Final *left = Node.getLeft();
    Expr *right = Node.getRight();

    left->accept(*this);

    if (right) {
      right->accept(*this);

      if (Node.getOperator() == Expr::Div || Node.getOperator() == Expr::Mod && right) {
        Final * f = (Final *)right;

        if (right && f->getKind() == Final::Number) {
          int intval;
          f->getVal().getAsInteger(10, intval);

          if (intval == 0) divide_by_zero_error();
        }
      }
    }
  };


  virtual void visit(Conditions &Node) override {
    llvm::errs() << "kiram to rafiee. " << "\n";
    Condition *left = Node.getLeft();
    llvm::errs() << "kiram to rafiee 2. " << "\n";

    Conditions *right = Node.getRight();
    llvm::errs() << "kiram to rafiee 3. " << "\n";

    left->accept(*this);
    llvm::errs() << "kiram to rafiee 4. " << "\n";

    if(right) right->accept(*this);
    llvm::errs() << "kiram to rafiee 5. " << "\n";

  };


  virtual void visit(Condition &Node) override {
    Expr *left = Node.getLeft();
    Expr *right = Node.getRight();

    left->accept(*this);
    right->accept(*this);
  };


  virtual void visit(Final &Node) override {
    if (Node.getKind() == Final::Ident) {
      // Check if identifier is in the scope
      if (Scope.find(Node.getVal()) == Scope.end())
        declare_error(Not, Node.getVal()); // Variable Not Found
    }
  };


  virtual void visit(If &Node) override {
    Conditions *conds = Node.getConds();
    Else *ElseBranch = Node.getElse();

    conds->accept(*this);

    for (llvm::SmallVector<Assign *>::const_iterator I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }
    
    for (llvm::SmallVector<Elif *>::const_iterator I = Node.ElifsBegin(), E = Node.ElifsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }

    if(ElseBranch) ElseBranch->accept(*this);

  };


  virtual void visit(Elif &Node) override {
    Conditions *conds = Node.getConds();

    conds->accept(*this);

    for (llvm::SmallVector<Assign *>::const_iterator I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }
  };


  virtual void visit(Else &Node) override {
    for (llvm::SmallVector<Assign *>::const_iterator I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) {
        (*I)->accept(*this);
    }
  };


  virtual void visit(Loop &Node) override {
    Conditions *conds = Node.getConds();

    conds->accept(*this);

    for (llvm::SmallVector<Assign *>::const_iterator I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) {
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