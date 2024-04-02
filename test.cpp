#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

int main()
{
    LLVMContext context;
    Module module("add_numbers", context);
    IRBuilder<> builder(context);

    // Create the main function
    FunctionType *funcType = FunctionType::get(builder.getInt32Ty(), false);
    Function *mainFunc = Function::Create(funcType, Function::ExternalLinkage, "main", module);

    BasicBlock *entry = BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // Add two numbers
    Value *num1 = ConstantInt::get(Type::getInt32Ty(context), 5);
    Value *num2 = ConstantInt::get(Type::getInt32Ty(context), 10);
    Value *sum = builder.CreateAdd(num1, num2, "sum");

    // Print the result
    FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), builder.getInt8PtrTy(), true);
    FunctionCallee printfFunc = module.getOrInsertFunction("printf", printfType);
    Function *printfFuncPtr = cast<Function>(printfFunc.getCallee());

    Value *formatStr = builder.CreateGlobalStringPtr("%d\n");
    builder.CreateCall(printfFunc, {formatStr, sum});

    builder.CreateRet(ConstantInt::get(Type::getInt32Ty(context), 0));

    // Verify the module
    verifyModule(module, &errs());

    // Print LLVM IR
    module.print(outs(), nullptr);

    return 0;
}
