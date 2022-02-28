#include "ast/FunctionAST.h"
#include "GetFunc.h"
// Generates LLVM code for functions declarations
llvm::Function *FunctionAST::codegen() {
 auto &P = *Proto;
 FunctionProtos[Proto->getName()] = std::move(Proto);
 Function *TheFunction = getFunction(P.getName());
 if(!TheFunction)


  if (!TheFunction) {
    TheFunction = Proto->codegen();
  }

  if (!TheFunction) {
    return nullptr;
  }

  llvm::BasicBlock *BB = llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
  Builder.SetInsertPoint(BB);
  NamedValues.clear();
  for (auto &Arg : TheFunction->args()) {
    NamedValues[Arg.getName()] = &Arg;
  }

  if (llvm::Value *RetVal = Body->codegen()) {
    Builder.CreateRet(RetVal);
    verifyFunction(*TheFunction);
          
       // Optimize the function.
        TheFPM->run(*TheFunction);

    return TheFunction;
  }

  TheFunction->eraseFromParent();
  return nullptr;
} 