#include <map>
#include <vector>
#include <string>
#include "llvm/ADT/StringRef.h"

#include "Optimizer.h"

namespace OptimizationMethods{
    class DetectDeadVars : public ASTVisitor {
    Value *V;
    std::map<StringRef, std::vector<StringRef>> variablesDependencyList;
    StringRef currentVar;

    public:
        void run(AST *Tree) {
            Tree->accept(*this);
        }
        
        virtual void visit(ARK &Node) override {

            if(Node.getKind() == Statement::Declaration)
        
            for (llvm::SmallVector<Statement *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
                (*I)->accept(*this); // Visit each child node
            }
        };

        //virtual void visit(Statement &Node) override {};

        virtual void visit(Declare &Node) override {
            Value *val = nullptr;
            
            StringRef currentVar = *(Node.VarsBegin());
            nameMap[Var] = Builder.CreateAlloca(Int32Ty);

            llvm::SmallVector<Expr *>::const_iterator L = Node.ExprsBegin();
            
            if (!L) {
                (*L)->accept(*this);
                // val = V;
                // if (val != nullptr) {
                //     Builder.CreateStore(val, nameMap[Var]);
                // }

                // ++L;
            }
        };

        virtual void visit(Assign &Node) override {
            // Get the name of the variable being assigned.
            currentVar = Node.getLeft()->getVal();

            // Visit the right-hand side of the assignment and get its value.
            Node.getRight()->accept(*this);
            //Value *val = V;   
        };

        virtual void visit(Expr &Node) override {
            // Visit the left-hand side of the binary operation and get its value.
            Node.getLeft()->accept(*this);
            Value *Left = V;

            // Visit the right-hand side of the binary operation and get its value.
            Value *Right;
            if(Node.getRight()) {
                Node.getRight()->accept(*this);
                Right = V;
            }
        };

        virtual void visit(Final &Node) override {
            if (Node.getKind() == Final::Ident) {
                variablesDependencyList[currentVar].push_back(Node.getVal());
            }
        };
    }
}

void Optimizer::optimize() {
    OptimizationMethods::DetectDeadVars *detectDeadVars = OptimizationMethods::DetectDeadVars;
    detectDeadVars->run();
    OptimizationMethods::RemoveDeadVars *removeDeadVars = OptimizationMethods::RemoveDeadVars;
    removeDeadVars->run();
}