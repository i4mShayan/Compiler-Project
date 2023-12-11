#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Define a visitor class for generating LLVM IR from the AST.
namespace
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
    }

    // Entry point for generating LLVM IR from the AST.
    void run(AST *Tree)
    {
      // Create the main function with the appropriate function type.
      FunctionType *MainFty = FunctionType::get(Int32Ty, {Int32Ty, Int8PtrPtrTy}, false);
      Function *MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);

      // Create a basic block for the entry point of the main function.
      BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
      Builder.SetInsertPoint(BB);

      // Visit the root node of the AST to generate IR.
      Tree->accept(*this);

      // Create a return instruction at the end of the main function.
      Builder.CreateRet(Int32Zero);
    }

    // Visit function for the MSM node in the AST.
    virtual void visit(MSM &Node) override
    {
      // Iterate over the children of the MSM node and visit each child.
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
    };

    virtual void visit(Statement &) override
    {
      // dont know what to do
    }

    virtual void visit(Assign &Node) override
    {
      // Visit the right-hand side of the assignment and get its value.
      Node.getRight()->accept(*this);
      Value *val = V;

      // Get the name of the variable being assigned.
      auto varName = Node.getLeft()->getVal();

      switch (Node.getAssignmentOP())
      {
      case Assign::EqualAssign:
        // Create a store instruction to assign the value to the variable.
        Builder.CreateStore(val, nameMap[varName]);

        // Create a function type for the "gsm_write" function.
        FunctionType *CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);

        // Create a function declaration for the "gsm_write" function.
        Function *CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "gsm_write", M);

        // Create a call instruction to invoke the "gsm_write" function with the value.
        CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});

        break;
      case Assign::PlusAssign:
        // Create a load instruction to get the current value of the variable.
        Value *oldVal = Builder.CreateLoad(Int32Ty, nameMap[varName]);

        // Create an add instruction to add the old value and the new value.
        Value *newVal = Builder.CreateNSWAdd(oldVal, val);

        // Create a store instruction to assign the new value to the variable.
        Builder.CreateStore(newVal, nameMap[varName]);

        // Create a function type for the "gsm_write" function.
        FunctionType *CalcWriteFnTy2 = FunctionType::get(VoidTy, {Int32Ty}, false);

        // Create a function declaration for the "gsm_write" function.
        Function *CalcWriteFn2 = Function::Create(CalcWriteFnTy2, GlobalValue::ExternalLinkage, "gsm_write", M);

        // Create a call instruction to invoke the "gsm_write" function with the new value.
        CallInst *Call2 = Builder.CreateCall(CalcWriteFnTy2, CalcWriteFn2, {newVal});

        break;
      case Assign::MinusAssign:
        // Create a load instruction to get the current value of the variable.
        Value *oldVal2 = Builder.CreateLoad(Int32Ty, nameMap[varName]);

        // Create a sub instruction to subtract the old value and the new value.
        Value *newVal2 = Builder.CreateNSWSub(oldVal2, val);

        // Create a store instruction to assign the new value to the variable.
        Builder.CreateStore(newVal2, nameMap[varName]);

        // Create a function type for the "gsm_write" function.
        FunctionType *CalcWriteFnTy3 = FunctionType::get(VoidTy, {Int32Ty}, false);

        // Create a function declaration for the "gsm_write" function.
        Function *CalcWriteFn3 = Function::Create(CalcWriteFnTy3, GlobalValue::ExternalLinkage, "gsm_write", M);

        // Create a call instruction to invoke the "gsm_write" function with the new value.
        CallInst *Call3 = Builder.CreateCall(CalcWriteFnTy3, CalcWriteFn3, {newVal2});

        break;
      case Assign::MulAssign:
        // Create a load instruction to get the current value of the variable.
        Value *oldVal3 = Builder.CreateLoad(Int32Ty, nameMap[varName]);

        // Create a mul instruction to multiply the old value and the new value.
        Value *newVal3 = Builder.CreateNSWMul(oldVal3, val);

        // Create a store instruction to assign the new value to the variable.
        Builder.CreateStore(newVal3, nameMap[varName]);

        // Create a function type for the "gsm_write" function.
        FunctionType *CalcWriteFnTy4 = FunctionType::get(VoidTy, {Int32Ty}, false);

        // Create a function declaration for the "gsm_write" function.
        Function *CalcWriteFn4 = Function::Create(CalcWriteFnTy4, GlobalValue::ExternalLinkage, "gsm_write", M);

        // Create a call instruction to invoke the "gsm_write" function with the new value.
        CallInst *Call4 = Builder.CreateCall(CalcWriteFnTy4, CalcWriteFn4, {newVal3});

        break;
      case Assign::DivAssign:
        // Create a load instruction to get the current value of the variable.
        Value *oldVal4 = Builder.CreateLoad(Int32Ty, nameMap[varName]);
        // Create a div instruction to divide the old value and the new value.
        Value *newVal4 = Builder.CreateSDiv(oldVal4, val);
        // Create a store instruction to assign the new value to the variable.
        Builder.CreateStore(newVal4, nameMap[varName]);
        // Create a function type for the "gsm_write" function.
        FunctionType *CalcWriteFnTy5 = FunctionType::get(VoidTy, {Int32Ty}, false);
        // Create a function declaration for the "gsm_write" function.
        Function *CalcWriteFn5 = Function::Create(CalcWriteFnTy5, GlobalValue::ExternalLinkage, "gsm_write", M);
        // Create a call instruction to invoke the "gsm_write" function with the new value.
        CallInst *Call5 = Builder.CreateCall(CalcWriteFnTy5, CalcWriteFn5, {newVal4});
        break;
      default:
        break;
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
      Node.getRight()->accept(*this);
      Value *Right = V;

      // Perform the binary operation based on the operator type and create the corresponding instruction.
      switch (Node.getOperator())
      {
      case Expr::Plus:
        V = Builder.CreateNSWAdd(Left, Right);
        break;
      case Expr::Minus:
        V = Builder.CreateNSWSub(Left, Right);
        break;
      case Expr::Mul:
        V = Builder.CreateNSWMul(Left, Right);
        break;
      case Expr::Div:
        V = Builder.CreateSDiv(Left, Right);
        break;
      case Expr::Pow:
        // handle for power
        // Create a function type for the "pow" function.
        FunctionType *PowFnTy = FunctionType::get(Int32Ty, {Int32Ty, Int32Ty}, false);

        // Create a function declaration for the "pow" function.
        Function *PowFn = Function::Create(PowFnTy, GlobalValue::ExternalLinkage, "pow", M);

        // Create a call instruction to invoke the "pow" function with the left and right values.
        CallInst *Call = Builder.CreateCall(PowFnTy, PowFn, {Left, Right});

        V = Call;
      }
    };

    virtual void visit(Declare &Node) override
    {
      Value *val = nullptr;

      // Iterate over the variables declared in the declaration statement.
      for (auto I = Node.getVars().begin(), g = Node.getExprs().begin(), E = Node.getVars().end(); I != E; ++I, ++g)
      {
        StringRef Var = *I;

        // Create an alloca instruction to allocate memory for the variable.
        nameMap[Var] = Builder.CreateAlloca(Int32Ty);

        if (g)
        {
          // If there is an expression provided, visit it and get its value.
          g->accept(*this);
          val = V;
        }

        // Store the initial value (if any) in the variable's memory location.
        if (val != nullptr)
        {
          Builder.CreateStore(val, nameMap[Var]);
        }
      }
    };

    virtula void visit(Condition &Node) override
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
        V = Left <= Right;
        break;
      case Condition::LessThan:
        V = Left < Right;
        break;
      case Condition::EqualEqual:
        V = Left == Right;
        break;
      case Condition::NotEqual:
        V = Left != Right;
        break;
      case Condition::GreaterThan:
        V = Left > Right;
        break;
      case Condition::GreaterEqual:
        V = Left >= Right;
        break;
      }
    }

    virtual void visit(If &Node)
    {
      Node.getCondition()->accept(*this);
      Value *val = V;
      if (V)
      {
        for (auto I = Node.getStatements().begin(), E = Node.getStatements().end(); I != E; ++I)
        {
          (*I)->accept(*this);
        }
      }
      else
      {
        V = 0;
        for (auto I = Node.getElifs().begin(), E = Node.getElifs().end(); I != E && V == 0; ++I)
        {
          (*I)->accept(*this);
        }
        if (V == 0)
        {
          Node.getElse()->accept(*this);
        }
      }
    }
  };
}; // namespace

void CodeGen::compile(AST *Tree)
{
  // Create an LLVM context and a module.
  LLVMContext Ctx;
  Module *M = new Module("calc.expr", Ctx);

  // Create an instance of the ToIRVisitor and run it on the AST to generate LLVM IR.
  ToIRVisitor ToIR(M);
  ToIR.run(Tree);

  // Print the generated module to the standard output.
  M->print(outs(), nullptr);
}
