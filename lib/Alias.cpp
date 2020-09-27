#include "Alias.h"

namespace AliasUtil {

void Alias::set(llvm::Value* Val, unsigned int Kind, int Index,
                llvm::Function* Func, bool Global) {
    this->Val = Val;
    this->Kind = Kind;
    this->Index = Index;
    this->Func = Func;
    this->IsGlobal = Global;
}

void Alias::set(llvm::Type* Ty, unsigned int Kind, int Index) {
    this->Ty = Ty;
    this->Kind = Kind;
    this->Index = Index;
}

void Alias::set(llvm::Argument* Arg, unsigned int Kind, int Index,
                llvm::Function* Func) {
    this->Arg = Arg;
    this->Kind = Kind;
    this->Index = Index;
    this->Func = Func;
}

void Alias::set(std::string S, unsigned int Kind, int Index,
                llvm::Function* Func) {
    this->name = S;
    this->Kind = Kind;
    this->Index = Index;
    if (!Func) this->IsGlobal = true;
    this->Func = Func;
}

Alias::Alias(llvm::Value* Val, int Index) {
    if (llvm::Argument* Arg = dyn_cast<llvm::Argument>(Val)) {
        set(Arg, /* Kind = */ 2, Index, Arg->getParent());
    } else {
        llvm::Function* func = nullptr;
        if (llvm::Instruction* Inst = dyn_cast<llvm::Instruction>(Val))
            func = Inst->getParent()->getParent();
        if (isa<llvm::GlobalVariable>(Val) || !func)
            set(Val, /* Kind = */ 0, Index, func, true);
        else
            set(Val, /* Kind = */ 0, Index, func);
    }
}

Alias::Alias(llvm::Argument* Arg, int Index) {
    set(Arg, /* Kind = */ 2, Index, Arg->getParent());
}

Alias::Alias(llvm::Type* Ty, int Index) { set(Ty, /* Kind = */ 1, Index); }

Alias::Alias(std::string S, llvm::Function* Func, int Index) {
    set(S, /* Kind = */ 3, Index, Func);
}

Alias::Alias(Alias* A) {
    unsigned int Kind = A->Kind;
    if (Kind == 0) {
        set(A->Val, A->Kind, A->Index, A->Func);
    } else if (Kind == 1) {
        set(A->Ty, A->Kind, A->Index);
    } else if (Kind == 2) {
        set(A->Arg, A->Kind, A->Index, A->Func);
    } else if (Kind == 3) {
        set(A->name, A->Kind, A->Index, A->Func);
    }
}

/// setIndex - For a GEP Instruction find the offset and store it
void Alias::setIndex(llvm::GetElementPtrInst* GEPInst) {
    auto IterRange = GEPInst->indices();
    auto Iter = IterRange.begin();
    int a = 0;
    while (Iter != IterRange.end()) {
        llvm::Value* temp = &(*Iter->get());
        // TODO: Can also use getValue of ConstantInt
        if (llvm::ConstantInt* CI = dyn_cast<llvm::ConstantInt>(temp))
            a = (a * 2) + CI->isOne();
        Iter++;
    }
    this->Index = a;
}

/// setIndex - For a GEP Operator find the offset and store it
void Alias::setIndex(llvm::GEPOperator* GEPOp) {
    auto Iter = GEPOp->idx_begin();
    int a = 0;
    while (Iter != GEPOp->idx_end()) {
        llvm::Value* temp = &(*Iter->get());
        if (llvm::ConstantInt* CI = dyn_cast<llvm::ConstantInt>(temp))
            a = (a * 2) + CI->isOne();
        Iter++;
    }
    this->Index = a;
}

/// getValue - Returns the underlying Value* for the alias
Value* Alias::getValue() const {
    if (this->Kind == 0) {
        return this->Val;
    }
    return nullptr;
}

/// print - Prints the alias
StringRef Alias::print() const {
    if (!IsGlobal && (Kind == 0 || Kind == 2 || Kind == 3)) {
        errs() << "[" << this->Func->getName() << "]"
               << " ";
    }
    if (Kind == 0) {
        errs() << this->Val->getName();
    } else if (Kind == 1) {
        std::string type_str;
        llvm::raw_string_ostream rso(type_str);
        this->Ty->print(rso);
        errs() << type_str;
    } else if (Kind == 2) {
        errs() << this->Arg->getName();
    } else if (Kind == 3) {
        errs() << this->name;
    }
    if (Index != -1) {
        errs() << "[" << Index << "]";
    }
    return "";
}

/// getName - Returns the name of alias with other informations like parent
/// function etc
StringRef Alias::getName() const {
    if (this->Kind == 0) {
        return this->Val->getName();
    } else if (this->Kind == 2) {
        return this->Arg->getName();
    } else if (this->Kind == 3) {
        return this->name;
    }
    return "";
}

/// isMem - Returns true if the alias denotes a location in heap
bool Alias::isMem() const { return this->Kind == 1; }

/// isGlobalVar - Returns true if the alias is global
bool Alias::IsGlobalVar() const { return this->Kind == 0 && this->IsGlobal; }

/// isArg - Returns true if alias is a function argument
bool Alias::isArg() const { return this->Kind == 2; }

/// isAllocaOrArgOrGlobal - Returns true if the alias is global, an argument or
/// alloca
bool Alias::isAllocaOrArgOrGlobal() const {
    return this->isMem() || this->IsGlobalVar() || this->isArg();
}

/// sameFunc = Returns true if the parent function of alias is same as /p Func
bool Alias::sameFunc(llvm::Function* Func) const {
    if (this->Func) return this->Func == Func;
    return false;
}

bool Alias::operator<(const Alias& TheAlias) const {
    bool less;
    if (Kind == 0) {
        less = this->Val < TheAlias.Val;
        less &= this->Func < TheAlias.Func;
    } else if (Kind == 1) {
        less = this->Ty < TheAlias.Ty;
    } else if (Kind == 2) {
        less = this->Arg < TheAlias.Arg;
        less &= this->Func < TheAlias.Func;
    } else if (Kind == 3) {
        less = this->name < TheAlias.name;
        less &= this->Func < TheAlias.Func;
    }
    return less && this->Index < TheAlias.Index;
}

bool Alias::operator==(const Alias& TheAlias) const {
    bool equal;
    if (Kind == 0) {
        equal = this->Val == TheAlias.Val;
        equal &= this->Func == TheAlias.Func;
    } else if (Kind == 1) {
        equal = this->Ty == TheAlias.Ty;
    } else if (Kind == 2) {
        equal = this->Arg == TheAlias.Arg;
        equal &= this->Func == TheAlias.Func;
    } else if (Kind == 3) {
        equal = this->name == TheAlias.name;
        equal &= this->Func == TheAlias.Func;
    }
    return equal && this->Index == TheAlias.Index;
}

void Alias::operator=(const Alias& TheAlias) {
    unsigned int Kind = TheAlias.Kind;
    if (Kind == 0) {
        set(TheAlias.Val, TheAlias.Kind, TheAlias.Index, TheAlias.Func);
    } else if (Kind == 1) {
        set(TheAlias.Ty, TheAlias.Kind, TheAlias.Index);
    } else if (Kind == 2) {
        set(TheAlias.Arg, TheAlias.Kind, TheAlias.Index, TheAlias.Func);
    } else if (Kind == 3) {
        set(TheAlias.name, TheAlias.Kind, TheAlias.Index, TheAlias.Func);
    }
}

}  // namespace AliasUtil
