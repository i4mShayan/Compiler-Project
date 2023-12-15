#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Define a visitor class for generating LLVM IR from the AST.
namespace ns
{
  class ToIRVisitor : public ASTVisitor
  {
    Module *M;
    IRBuilder<> Builder;
    Type *VoidTy;
    Type *Int32Ty;
    Type *Int8PtrTy;
    Type *Int8PtrPtrTy;
    Constant *Int32Zero;

    Value *V;
    StringMap<AllocaInst *> nameMap;

    llvm::FunctionType* MainFty;
    llvm::Function* MainFn;

    FunctionType *CalcWriteFnTy;
    Function *CalcWriteFn;

    llvm::BasicBlock* LoopCond;
    llvm::BasicBlock* LoopBody;
    llvm::BasicBlock* AfterLoop;
  public:
    // Constructor for the visitor class.
    ToIRVisitor(Module *M) : M(M), Builder(M->getContext())
    {
      // Initialize LLVM types and constants.
      VoidTy = Type::getVoidTy(M->getContext());
      Int32Ty = Type::getInt32Ty(M->getContext());
      Int8PtrTy = Type::getInt8PtrTy(M->getContext());
      Int8PtrPtrTy = Int8PtrTy->getPointerTo();
      Int32Zero = ConstantInt::get(Int32Ty, 0, true);

      // Create a function type for the "gsm_write" function.
      CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);
      // Create a function declaration for the "gsm_write" function.
      CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "ark_write", M);
    }

    // Entry point for generating LLVM IR from the AST.
    void run(AST *Tree)
    {
      // Create the main function with the appropriate function type.
      FunctionType *MainFty = FunctionType::get(Int32Ty, {Int32Ty, Int8PtrPtrTy}, false);
      Function *MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);

      // // Create a basic block for the entry point of the main function.
      // BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
      // Builder.SetInsertPoint(BB);


      LoopCond = llvm::BasicBlock::Create(M->getContext(), "loop.cond", MainFn);
      LoopBody = llvm::BasicBlock::Create(M->getContext(), "loop.body", MainFn);
      AfterLoop = llvm::BasicBlock::Create(M->getContext(), "after.loop", MainFn);

      // Visit the root node of the AST to generate IR.
      Tree->accept(*this);

      // Create a return instruction at the end of the main function.
      Builder.CreateRet(Int32Zero);

    }

    // Visit function for the ARK node in the AST.
    virtual void visit(ARK &Node) override
    {
      // Iterate over the children of the MSM node and visit each child.
      for (llvm::SmallVector<Statement *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
    };

    virtual void visit(Statement &Node) override
    {
      // Statement *pointer = &Node;
      // if (Node.getKind() == Statement::Assignment)
      // {
      //   Assign *assign = static_cast<Assign*>(pointer);
      //   assign->accept(*this);
      // }
      // else if (Node.getKind() == Statement::Declaration)
      // {
      //   Declare *declare = static_cast<Declare*>(pointer);
      //   declare->accept(*this);
      // }
      // else if (Node.getKind() == Statement::If)
      // {
      //   If *if_stm = static_cast<If*>(pointer);
      //   if_stm->accept(*this);
      // }
      // else if (Node.getKind() == Statement::Loop)
      // {
      //   Loop *loop = static_cast<Loop*>(pointer);
      //   loop->accept(*this);
      // }
    
    };


virtual void visit(Assign &Node) override
    {
      // Visit the right-hand side of the assignment and get its value.
      Node.getRight()->accept(*this);
      Value *val = V;

      // Get the name of the variable being assigned.
      llvm::StringRef varName = Node.getLeft()->getVal();

      switch (Node.getAssignmentOP())
      {
      case Assign::EqualAssign:
      {
        // Create a store instruction to assign the value to the variable.
        Builder.CreateStore(val, nameMap[varName]);

        // Create a call instruction to invoke the "gsm_write" function with the value.
        Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});

        break;
      }
      case Assign::PlusAssign :
      {
        // Create a load instruction to get the current value of the variable.
        Value *oldVal = Builder.CreateLoad(Int32Ty, nameMap[varName]);

        // Create an add instruction to add the old value and the new value.
        Value *newVal = Builder.CreateNSWAdd(oldVal, val);

        // Create a store instruction to assign the new value to the variable.
        Builder.CreateStore(newVal, nameMap[varName]);

        // Create a call instruction to invoke the "gsm_write" function with the new value.
        CallInst *Call2 = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {newVal});

        break;
      }
      case Assign::MinusAssign:
      {
        // Create a load instruction to get the current value of the variable.
        Value *oldVal2 = Builder.CreateLoad(Int32Ty, nameMap[varName]);

        // Create a sub instruction to subtract the old value and the new value.
        Value *newVal2 = Builder.CreateNSWSub(oldVal2, val);

        // Create a store instruction to assign the new value to the variable.
        Builder.CreateStore(newVal2, nameMap[varName]);

        // Create a call instruction to invoke the "gsm_write" function with the new value.
        CallInst *Call3 = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {newVal2});

        break;
      }
      case Assign::MulAssign:
        {
        // Create a load instruction to get the current value of the variable.
        Value *oldVal3 = Builder.CreateLoad(Int32Ty, nameMap[varName]);

        // Create a mul instruction to multiply the old value and the new value.
        Value *newVal3 = Builder.CreateNSWMul(oldVal3, val);

        // Create a store instruction to assign the new value to the variable.
        Builder.CreateStore(newVal3, nameMap[varName]);

        // Create a call instruction to invoke the "gsm_write" function with the new value.
        CallInst *Call4 = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {newVal3});

        break;
        }
      case Assign::DivAssign:
      {
        // Create a load instruction to get the current value of the variable.
        Value *oldVal4 = Builder.CreateLoad(Int32Ty, nameMap[varName]);
        // Create a div instruction to divide the old value and the new value.
        Value *newVal4 = Builder.CreateSDiv(oldVal4, val);
        // Create a store instruction to assign the new value to the variable.
        Builder.CreateStore(newVal4, nameMap[varName]);
        // Create a call instruction to invoke the "gsm_write" function with the new value.
        CallInst *Call5 = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {newVal4});
        break;
      }
      case Assign::ModAssign:
      {
        Value *oldVal5 = Builder.CreateLoad(Int32Ty, nameMap[varName]);
        Value *newVal5 = Builder.CreateSRem(oldVal5, val);
        Builder.CreateStore(newVal5, nameMap[varName]);
        CallInst *Call6 = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {newVal5});
        break;
      }
      }
    };

    virtual void visit(Final &Node) override
    {
      if (Node.getKind() == Final::Ident)
      {
        // If the factor is an identifier, load its value from memory.
        V = Builder.CreateLoad(Int32Ty, nameMap[Node.getVal()]);
      }
      else
      {
        // If the factor is a literal, convert it to an integer and create a constant.
        int intval;
        Node.getVal().getAsInteger(10, intval);
        V = ConstantInt::get(Int32Ty, intval, true);
      }
    };

    virtual void visit(Expr &Node) override
    {
      // Visit the left-hand side of the binary operation and get its value.
      Node.getLeft()->accept(*this);
      Value *Left = V;

      // Visit the right-hand side of the binary operation and get its value.
      Value *Right;
      if(Node.getRight()) {
        Node.getRight()->accept(*this);
        Right = V;

        // Perform the binary operation based on the operator type and create the corresponding instruction.
        switch (Node.getOperator())
        {
          case Expr::Plus:
          {
            V = Builder.CreateNSWAdd(Left, Right);
            break;
          }
          case Expr::Minus:
          {
            V = Builder.CreateNSWSub(Left, Right);
            break;
          }
          case Expr::Mul:
          {
            V = Builder.CreateNSWMul(Left, Right);
            break;
          }
          case Expr::Div:
          {
            V = Builder.CreateSDiv(Left, Right);
            break;
          }
          case Expr::Pow:
          {
            Final *right_final = (Final *) Node.getRight();
            int intval = 0;
            right_final->getVal().getAsInteger(10, intval);
            if (intval == 0)
            {
              V = ConstantInt::get(Int32Ty, 1, true);
            }
            else
            {
              
              Value* TempLeft = Left;
              for (int i = 1; i < intval; i++)
              {
                Left = Builder.CreateNSWMul(Left, TempLeft);
              }
              V = Left;
            }
            break;
          }
          case Expr::Mod:
          {
            // x % y = x - (x / y) * y
            Value *division = Builder.CreateSDiv(Left, Right);
            Value *multiply = Builder.CreateNSWMul(division, Right);
            V = Builder.CreateNSWSub(Left, multiply);
            break;
          }
        }
      }  
    };

    virtual void visit(Declare &Node) override
    {
      Value *val = nullptr;

      llvm::SmallVector<Expr *>::const_iterator L = Node.ExprsBegin();
      llvm::SmallVector<Expr *>::const_iterator R = Node.ExprsEnd();
      for (llvm::SmallVector<llvm::StringRef, 8>::const_iterator I = Node.VarsBegin(), E = Node.VarsEnd(); I != E; ++I)
      {
        StringRef Var = *I;
        nameMap[Var] = Builder.CreateAlloca(Int32Ty);

        if (L != R)
        {
          (*L)->accept(*this);
          val = V;
          if (val != nullptr)
          {
            Builder.CreateStore(val, nameMap[Var]);
          }

          ++L;
        }
        else
        {
          Builder.CreateStore(Int32Zero, nameMap[Var]);
        }
      }
    };

    virtual void visit(Condition &Node) override
    {

      Node.getLeft()->accept(*this);
      Value *Left = V;

      // Visit the right-hand side of the binary operation and get its value.
      Node.getRight()->accept(*this);
      Value *Right = V;
      
      // Perform the binary operation based on the operator type and create the corresponding instruction.
      switch (Node.getSign())
      {
        case Condition::LessEqual:
        {
          V = Builder.CreateICmpSLE(Left, Right);
          break;
        }
        case Condition::LessThan:
        {
          V = Builder.CreateICmpSLT(Left, Right);
          break;
        }
        case Condition::GreaterThan:
        {
          V = Builder.CreateICmpSGT(Left, Right);
          break;
        }
        case Condition::GreaterEqual:
        {
          V = Builder.CreateICmpSGE(Left, Right);
          break;
        }
        case Condition::EqualEqual:
        {
          V = Builder.CreateICmpEQ(Left, Right);
          break;
        }
        case Condition::NotEqual:
        {
          V = Builder.CreateICmpNE(Left, Right);
          break;
        }
      }
    };

    virtual void visit(Conditions &Node) override
    {
      Node.getLeft()->accept(*this);
      Value* Left = V;
      
      if (!Node.getRight())
      {
        V = Left;
        return;
      }
      
      Node.getRight()->accept(*this);
      Value* Right = V;

      switch (Node.getSign())
      {
      case Conditions::And:
          V = Builder.CreateAnd(Left, Right);
          break;
      case Conditions::Or:
          V = Builder.CreateOr(Left, Right);
          break;
      }
    };

    virtual void visit(If &Node) override
    {
      llvm::BasicBlock* IfCond = llvm::BasicBlock::Create(M->getContext(), "if.cond", MainFn);
      llvm::BasicBlock* IfBody = llvm::BasicBlock::Create(M->getContext(), "if.body", MainFn);
      llvm::BasicBlock* AfterIf = llvm::BasicBlock::Create(M->getContext(), "after.if", MainFn);

      Builder.SetInsertPoint(IfCond);
      Node.getConds()->accept(*this);
      Value* IfCondVal = V;

      Builder.SetInsertPoint(IfBody);
      
      for (llvm::SmallVector<Assign *>::const_iterator I = Node.AssignmentsBegin(), E = Node.AssignmentsBegin(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
      Builder.CreateBr(AfterIf);
           

      llvm::BasicBlock* PrevCond = IfCond;
      llvm::BasicBlock* PrevBody = IfBody;
      Value* PrevCondVal = IfCondVal;

      for (llvm::SmallVector<Elif *>::const_iterator I = Node.ElifsBegin(), E = Node.ElifsEnd(); I != E; ++I) {
        llvm::BasicBlock* ElifCond = llvm::BasicBlock::Create(M->getContext(), "elif.cond", MainFn);
        llvm::BasicBlock* ElifBody = llvm::BasicBlock::Create(M->getContext(), "elif.body", MainFn);

        Builder.SetInsertPoint(PrevCond); 
        Builder.CreateCondBr(PrevCondVal, PrevBody, ElifCond);

        Builder.SetInsertPoint(ElifCond);
        (*I)->getConds()->accept(*this);
        llvm::Value* ElifCondVal = V;
        // Builder.CreateCondBr(ElifCondVal, ElifBody, nullptr);

        Builder.SetInsertPoint(ElifBody);
        (*I)->accept(*this);
        Builder.CreateBr(AfterIf);

        PrevCond = ElifCond;
        PrevCondVal = ElifCondVal;
        PrevBody = ElifBody;
      }

      llvm::BasicBlock* ElseBody = nullptr;
      if (Node.getElse()) {
          ElseBody = llvm::BasicBlock::Create(M->getContext(), "else.body", MainFn);
          Builder.SetInsertPoint(ElseBody);
          Node.getElse()->accept(*this);
          Builder.CreateBr(AfterIf);

          Builder.SetInsertPoint(PrevCond);
          Builder.CreateCondBr(PrevCondVal, PrevBody, ElseBody);
      } else {
          Builder.SetInsertPoint(PrevCond);
          Builder.CreateCondBr(IfCondVal, PrevBody, AfterIf);
      }

      Builder.SetInsertPoint(AfterIf);
      
    };

    virtual void visit(Elif &Node) override
    {
      for (llvm::SmallVector<Assign *>::const_iterator I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I)
      {
          (*I)->accept(*this);
      }
    };

    virtual void visit(Else &Node) override
    {
      for (llvm::SmallVector<Assign *>::const_iterator I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I)
      {
          (*I)->accept(*this);
      }
    };


    virtual void visit(Loop &Node) override
    {
      // llvm::BasicBlock* LoopCond = llvm::BasicBlock::Create(M->getContext(), "loop.cond", MainFn);
      // llvm::BasicBlock* LoopBody = llvm::BasicBlock::Create(M->getContext(), "loop.body", MainFn);
      // llvm::BasicBlock* AfterLoop = llvm::BasicBlock::Create(M->getContext(), "after.loop", MainFn);

      // Builder.CreateBr(LoopCond); 
      // Builder.SetInsertPoint(LoopCond); 
      Node.getConds()->accept(*this); 
      Value* Cond = V; 
      llvm::errs() << "Loop Condition is ";
      Cond->print(llvm::errs());
      llvm::errs() << "\n-----------\n";
      // Builder.SetInsertPoint(LoopBody);
      // Builder.CreateCondBr(Cond, LoopBody, AfterLoop); 

      for (llvm::SmallVector<Assign *>::const_iterator I = Node.AssignmentsBegin(), E = Node.AssignmentsEnd(); I != E; ++I) 
      {
          (*I)->accept(*this); 
      }
      // Builder.CreateBr(LoopCond); 

      // Builder.SetInsertPoint(AfterLoop);
    };
  };
}; // namespace

void CodeGen::compile(AST *Tree)
{
  // Create an LLVM context and a module.
  LLVMContext Ctx;
  Module *M = new Module("ark", Ctx);

  // Create an instance of the ToIRVisitor and run it on the AST to generate LLVM IR.
  // ToIRVisitor ToIR(M);
  ns::ToIRVisitor *ToIR = new ns::ToIRVisitor(M);
  
  ToIR->run(Tree);

  // Print the generated module to the standard output.
  M->print(outs(), nullptr);
}