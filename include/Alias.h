#ifndef ALIASANALYSIS_H
#define ALIASANALYSIS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "string"

using namespace llvm;

namespace AliasUtil {

class Alias {
   private:
    Value* Val;
    Type* Ty;
    Argument* Arg;
    // 0 is Value
    // 1 is Type
    // 2 is Argument
    // 3 is Dummy
    int Index;
    unsigned int Kind;
    Function* Func;
    std::string name;
    bool IsGlobal;

    void set(Value* Val, unsigned int Kind, int Index, Function* Func,
             bool Global = false);
    void set(Type* Ty, unsigned int Kind, int Index);
    void set(Argument* Arg, unsigned int Kind, int Index, Function* Func);
    void set(std::string S, unsigned int Kind, int Index, Function* Func);

    void setIndex(GetElementPtrInst* GEPInst);
    void setIndex(GEPOperator* GEPOp);
   public:
    Alias(Value* Val, int Index = -1);
    Alias(Argument* Arg, int Index = -1);
    Alias(Type* Ty, int Index = -1);
    Alias(std::string S, Function* Func, int Index = -1);
    Alias(Alias* A);

    Value* getValue();
    StringRef getName();

    StringRef print();

    bool isMem();
    bool isArg();
    bool IsGlobalVar();
    bool isAllocaOrArgOrGlobal();
    bool sameFunc(Function* Func);

    bool operator<(const Alias& TheAlias) const;
    bool operator==(const Alias& TheAlias) const;
};
}  // namespace AliasUtil

#endif
