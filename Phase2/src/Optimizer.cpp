#include <map>
#include <vector>
#include <string>
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Value.h"

#include "Optimizer.h"

using namespace llvm;

bool debugMode;

namespace OptimizationMethods{
    std::map<StringRef, std::vector<StringRef>> variablesDependencyList;
    std::vector<StringRef> liveVars;


    // ------------------- DetectDeadVars Class-------------------
    class DetectDeadVars : public ASTVisitor {
        StringRef currentVar;
        StringRef goalVar;

    public:
        void debug() {
            llvm::errs() << "\n--------------------\n";
            for (const auto& pair : variablesDependencyList) {
                const StringRef& key = pair.first;
                const std::vector<StringRef>& list = pair.second;

                llvm::errs() << "Var: " << key << "\n";
                llvm::errs() << "DependedOn: ";
                for (StringRef var : list) {
                    llvm::errs() << var << " ";
                }
                llvm::errs() << "\n--------------------\n";
            }

            llvm::errs() << "LiveVars: ";

            for (const auto& liveVar : liveVars) {
                llvm::errs() << liveVar << " ";
            }
            llvm::errs() << "\n--------------------\n\n";
        }

        void findLiveVars(StringRef Var) {      
            auto dependedOnVarsMap = variablesDependencyList.find(Var);
            if(dependedOnVarsMap == variablesDependencyList.end()) return;

            std::vector<StringRef>& dependedOnVarsList = dependedOnVarsMap->second;

            for (const auto& dVar : dependedOnVarsList) {
                auto iter = std::find(liveVars.begin(), liveVars.end(), dVar);
                if (iter == liveVars.end()) {
                    liveVars.push_back(dVar);
                }
                if(dVar != goalVar && !dVar.equals(Var)) {
                    findLiveVars(dVar);
                }
            }
        }

        void run(AST *Tree, std::string goalVarName) {
            Tree->accept(*this);

            llvm::StringRef goalVarStringRef(goalVarName);
            goalVar = goalVarStringRef;

            liveVars.clear();
            liveVars.push_back(goalVar);
            findLiveVars(goalVar);

            if(debugMode) {
                debug();
            };
        }
        
        virtual void visit(ARK &Node) override {
            for (SmallVector<Statement *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
                (*I)->accept(*this); // Visit each child node
            }
        };

        virtual void visit(Declare &Node) override {
            Value *val = nullptr;
            
            currentVar = *(Node.VarsBegin());
            if (variablesDependencyList.count(currentVar) > 0) {
                // Clear the list associated with the currentVar key
                variablesDependencyList[currentVar].clear();
            }

            llvm::SmallVector<Expr *>::const_iterator L = Node.ExprsBegin();
            llvm::SmallVector<Expr *>::const_iterator R = Node.ExprsEnd();

            if (L != R) {
                (*L)->accept(*this);
            }
        };

        virtual void visit(Assign &Node) override {
            // Get the name of the variable being assigned.
            currentVar = Node.getLeft()->getVal();
            
            if (Node.getAssignmentOP() == Assign::EqualAssign && variablesDependencyList.count(currentVar) > 0) {
                // Clear the list associated with the currentVar key
                variablesDependencyList[currentVar].clear();
            }

            // Visit the right-hand side of the assignment and get its value.
            Node.getRight()->accept(*this);   
        };

        virtual void visit(Expr &Node) override {
            // Visit the left-hand side of the binary operation and get its value.
            Node.getLeft()->accept(*this);

            // Visit the right-hand side of the binary operation and get its value.
            Value *Right;
            if(Node.getRight()) {
                Node.getRight()->accept(*this);
            }
        };

        virtual void visit(Final &Node) override {
            if (Node.getKind() == Final::Ident) {
                variablesDependencyList[currentVar].push_back(Node.getVal());
            }
        };
    };

    // ------------------- RemoveDeadVars Class-------------------
    class RemoveDeadVars : public ASTVisitor {
        StringRef currentVar;
        std::vector<Statement*> statementsToRemove;

    public:
        void debug(ARK &Node) {
            for (SmallVector<Statement *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
                (*I)->accept(*this); // Visit each child node
            }
        }

        void run(AST *Tree) {
            Tree->accept(*this);
        }
        
        virtual void visit(ARK& Node) override {
            if(debugMode) {
                llvm::errs() << "*********** Removed Variables with their Declaration or Assignments: ***********\n";
            }

            for (SmallVector<Statement*>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
                (*I)->accept(*this); // Visit each child node

                auto iter = std::find(liveVars.begin(), liveVars.end(), currentVar);
                if (iter == liveVars.end()) {
                    // Add the current item to the removal list
                    statementsToRemove.push_back(*I);

                    if (debugMode) {
                        llvm::errs() << "\tRemoved -> " << currentVar << "\n";
                    }
                }
            }

            if(debugMode) {
                llvm::errs() << "********************************************************************************\n\n";
            }

            // Remove the statements marked for removal
            for (Statement* stmt : statementsToRemove) {
                Node.erase(stmt);
            }
        };

        virtual void visit(Declare &Node) override {
            Value *val = nullptr;
            
            currentVar = *(Node.VarsBegin());
        };

        virtual void visit(Assign &Node) override {
            // Get the name of the variable being assigned.
            currentVar = Node.getLeft()->getVal();
        };

        virtual void visit(Expr &Node) override {};

        virtual void visit(Final &Node) override {};
    };
}

void Optimizer::optimize(AST *Tree, bool enableDebugMode) {
    debugMode = enableDebugMode;
    std::string goalVar = "result";
    OptimizationMethods::DetectDeadVars *detectDeadVars = new OptimizationMethods::DetectDeadVars();
    detectDeadVars->run(Tree, goalVar);

    OptimizationMethods::RemoveDeadVars *removeDeadVars = new OptimizationMethods::RemoveDeadVars();
    removeDeadVars->run(Tree);
}