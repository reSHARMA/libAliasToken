#include "Alias.h"

using namespace llvm;
namespace AliasUtil {

void Alias::set(Value* Val, unsigned int Kind, int Index, Function* Func,
                bool Global) {
    this->Val = Val;
    this->Kind = Kind;
    this->Index = Index;
    this->Func = Func;
    this->IsGlobal = Global;
}

void Alias::set(Type* Ty, unsigned int Kind, int Index) {
    this->Ty = Ty;
    this->Kind = Kind;
    this->Index = Index;
}

void Alias::set(Argument* Arg, unsigned int Kind, int Index, Function* Func) {
    this->Arg = Arg;
    this->Kind = Kind;
    this->Index = Index;
    this->Func = Func;
}

void Alias::set(std::string S, unsigned int Kind, int Index, Function* Func) {
    this->name = S;
    this->Kind = Kind;
    this->Index = Index;
    if (!Func) this->IsGlobal = true;
    this->Func = Func;
}

Alias::Alias(Value* Val, int Index) {
    if (Argument* Arg = dyn_cast<Argument>(Val)) {
        set(Arg, /* Kind = */ 2, Index, Arg->getParent());
    } else {
        Function* func = nullptr;
        if (Instruction* Inst = dyn_cast<Instruction>(Val))
            func = Inst->getParent()->getParent();
        if (isa<GlobalVariable>(Val) || !func)
            set(Val, /* Kind = */ 0, Index, func, true);
        else
            set(Val, /* Kind = */ 0, Index, func);
    }
}

Alias::Alias(Argument* Arg, int Index) {
    set(Arg, /* Kind = */ 2, Index, Arg->getParent());
}

Alias::Alias(Type* Ty, int Index) { set(Ty, /* Kind = */ 1, Index); }

Alias::Alias(std::string S, Function* Func, int Index) {
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
void Alias::setIndex(GetElementPtrInst* GEPInst) {
    auto IterRange = GEPInst->indices();
    auto Iter = IterRange.begin();
    int a = 0;
    while (Iter != IterRange.end()) {
        Value* temp = &(*Iter->get());
        // TODO: Can also use getValue of ConstantInt
        if (ConstantInt* CI = dyn_cast<ConstantInt>(temp))
            a = (a * 2) + CI->isOne();
        Iter++;
    }
    this->Index = a;
}

/// setIndex - For a GEP Operator find the offset and store it
void Alias::setIndex(GEPOperator* GEPOp) {
    auto Iter = GEPOp->idx_begin();
    int a = 0;
    while (Iter != GEPOp->idx_end()) {
        Value* temp = &(*Iter->get());
        if (ConstantInt* CI = dyn_cast<ConstantInt>(temp))
            a = (a * 2) + CI->isOne();
        Iter++;
    }
    this->Index = a;
}

/// getValue - Returns the underlying Value* for the alias
Value* Alias::getValue() {
    if (this->Kind == 0) {
        return this->Val;
    }
    return nullptr;
}

/// print - Prints the alias
StringRef Alias::print() {
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
StringRef Alias::getName() {
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
bool Alias::isMem() { return this->Kind == 1; }

/// isGlobalVar - Returns true if the alias is global
bool Alias::IsGlobalVar() { return this->Kind == 0 && this->IsGlobal; }

/// isArg - Returns true if alias is a function argument
bool Alias::isArg() { return this->Kind == 2; }

/// isAllocaOrArgOrGlobal - Returns true if the alias is global, an argument or
/// alloca
bool Alias::isAllocaOrArgOrGlobal() {
    return this->isMem() || this->IsGlobalVar() || this->isArg();
}

/// sameFunc = Returns true if the parent function of alias is same as /p Func
bool Alias::sameFunc(Function* Func) {
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
}  // namespace AliasUtil
