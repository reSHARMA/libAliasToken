#ifndef ALIAS_H
#define ALIAS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "string"

namespace AliasUtil {

class Alias {
   private:
    llvm::Value* Val;
    llvm::Type* Ty;
    llvm::Argument* Arg;
    // 0 is Value
    // 1 is Type
    // 2 is Argument
    // 3 is Dummy
    int Index;
    unsigned int Kind;
    llvm::Function* Func;
    std::string name;
    bool IsGlobal;

    void set(llvm::Value* Val, unsigned int Kind, int Index,
             llvm::Function* Func, bool Global = false);
    void set(llvm::Type* Ty, unsigned int Kind, int Index);
    void set(llvm::Argument* Arg, unsigned int Kind, int Index,
             llvm::Function* Func);
    void set(std::string S, unsigned int Kind, int Index, llvm::Function* Func);

   public:

    void setIndex(llvm::GetElementPtrInst* GEPInst);
    void setIndex(llvm::GEPOperator* GEPOp);

    Alias(llvm::Value* Val, int Index = -1);
    Alias(llvm::Argument* Arg, int Index = -1);
    Alias(llvm::Type* Ty, int Index = -1);
    Alias(std::string S, llvm::Function* Func, int Index = -1);
    Alias(Alias* A);

    llvm::Value* getValue() const;
    llvm::StringRef getName() const;
    std::string getMemTypeName() const;
    std::string getFunctionName() const;
    int getFieldIndex() const;

    friend std::ostream& operator<<(std::ostream& OS, const Alias &A);

    bool isMem() const;
    bool isArg() const;
    bool isField() const;
    bool isGlobalVar() const;
    bool isAllocaOrArgOrGlobal() const;
    bool sameFunc(llvm::Function* Func) const;

    bool operator<(const Alias& TheAlias) const;
    bool operator==(const Alias& TheAlias) const;
    void operator=(const Alias& TheAlias);
};
}  // namespace AliasUtil

#endif
