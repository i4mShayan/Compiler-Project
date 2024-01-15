#include "Optimizer.h"


namespace OptimizationMethods{
    class RemoveDeadValues : public ASTVisitor {
    public:
          virtual void visit(ARK &Node) override {
            if(Node.getKind() == Statement::Declaration)
            
            for (llvm::SmallVector<Statement *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
            {
                (*I)->accept(*this); // Visit each child node
            }
          };

          virtual void visit(Statement &Node) override {};
          virtual void visit(Declare &Node) override {};
          virtual void visit(Assign &Node) override {};
          virtual void visit(Final &Node) override {};
    }
}

void Optimizer::optimize() {
    OptimizationMethods::RemoveDeadValues
}