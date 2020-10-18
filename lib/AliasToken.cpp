#include "AliasToken.h"

namespace AliasUtil {

///  getHash - Calculates the hash for alias to avoid multiple enteries of same
///  alias
std::string AliasTokens::getHash(Alias* A) {
    std::string hash = "";
    if (A->isGlobalVar()) hash += "G";
    hash += A->getName().str();
    hash += A->getFunctionName();
    hash += A->getMemTypeName();
    hash += std::to_string(A->getFieldIndex());
    return hash;
}

/// isCached - Returns true if Alias \p A is already present in cache
bool AliasTokens::isCached(Alias* A) {
    std::string hash = getHash(A);
    return AliasBank.find(hash) != AliasBank.end();
}

/// insert - Returns true after inserting the Alias \p A in cache, retuns false
/// if it is already present
bool AliasTokens::insert(Alias* A) {
    if (isCached(A)) {
        delete A;
        return false;
    } else {
        std::string hash = getHash(A);
        AliasBank[hash] = A;
        return true;
    }
}

/// getAliasToken - Returns Alias object for Value \p Val, returns the object
/// from cache if it already exists
Alias* AliasTokens::getAliasToken(llvm::Value* Val) {
    Alias* A = new Alias(Val);
    std::string hash = getHash(A);
    if (insert(A)) return A;
    return AliasBank[hash];
}

/// getAliasToken - Returns Alias object for Argument \p Arg, returns the object
/// from cache if it already exists
Alias* AliasTokens::getAliasToken(llvm::Argument* Arg) {
    Alias* A = new Alias(Arg);
    std::string hash = getHash(A);
    if (insert(A)) return A;
    return AliasBank[hash];
}

/// getAliasToken - Returns Alias object for Type \p Ty, returns the object from
/// cache if it already exists
Alias* AliasTokens::getAliasToken(llvm::Type* Ty) {
    Alias* A = new Alias(Ty);
    std::string hash = getHash(A);
    if (insert(A)) return A;
    return AliasBank[hash];
}

/// getAliasToken - Returns Alias object for Instruction \p Inst, returns the
/// object from cache if it already exists
Alias* AliasTokens::getAliasToken(llvm::Instruction* Inst) {
    Alias* A = new Alias(Inst);
    std::string hash = getHash(A);
    if (insert(A)) return A;
    return AliasBank[hash];
}

/// getAliasToken - Returns Alias object from another alias object \p A, returns
/// the object from cache if it already exists
Alias* AliasTokens::getAliasToken(Alias* A) {
    std::string hash = getHash(A);
    if (insert(A)) return A;
    return AliasBank[hash];
}

/// getAliasToken - Returns Alias object for String \p S, returns the object
/// from cache if it already exists.
///
/// \S is the name of dummy alias
/// \Func is the function associated with the alias object, pass nullptr if the
/// dummy oject at a global scope
Alias* AliasTokens::getAliasToken(std::string S, llvm::Function* Func) {
    Alias* A = new Alias(S, Func);
    std::string hash = getHash(A);
    if (insert(A)) return A;
    return AliasBank[hash];
}

/// extractAliasToken - Returns a vector of alias objects derived from
/// Instruction \Inst operands
std::vector<Alias*> AliasTokens::extractAliasToken(llvm::Instruction* Inst) {
    if (llvm::StoreInst* SI = llvm::dyn_cast<llvm::StoreInst>(Inst)) {
        return extractAliasToken(SI);
    } else if (llvm::LoadInst* LI = llvm::dyn_cast<llvm::LoadInst>(Inst)) {
        return extractAliasToken(LI);
    } else if (llvm::AllocaInst* AI = llvm::dyn_cast<llvm::AllocaInst>(Inst)) {
        return extractAliasToken(AI);
    } else if (llvm::BitCastInst* BI =
                   llvm::dyn_cast<llvm::BitCastInst>(Inst)) {
        return extractAliasToken(BI);
    } else if (llvm::ReturnInst* RI = llvm::dyn_cast<llvm::ReturnInst>(Inst)) {
        return extractAliasToken(RI);
    } else {
        // Direct support to some instructions may not be useful example
        // CallInst, as it is more useful to generate alias object for call
        // arguments on the fly
        llvm::errs() << "[TODO]: Unsupported Instruction " << *Inst << "\n";
    }
    return {};
}

/// extractAliasToken - Returns a vector of alias objects for StoreInst \Inst
/// operands.
std::vector<Alias*> AliasTokens::extractAliasToken(llvm::StoreInst* Inst) {
    // The operands are returned in the same order as they are present in the
    // instruction example store op1 op2
    std::vector<Alias*> AliasVec;
    llvm::Value* ValOp = Inst->getValueOperand();
    if (!llvm::isa<llvm::ConstantInt>(ValOp))
        AliasVec.push_back(this->getAliasToken(ValOp));
    AliasVec.push_back(this->getAliasToken(Inst->getPointerOperand()));
    return AliasVec;
}

/// extractAliasToken - Returns a vector of alias objects for StoreInst \Inst
/// operands.
std::vector<Alias*> AliasTokens::extractAliasToken(llvm::LoadInst* Inst) {
    // The operands are returned in the same order as they are present in the
    // instruction example x = load op1
    std::vector<Alias*> AliasVec;
    AliasVec.push_back(this->getAliasToken(Inst));
    AliasVec.push_back(this->getAliasToken(Inst->getPointerOperand()));
    return AliasVec;
}

/// extractAliasToken - Returns a vector of alias objects for AllocaInst \Inst
/// operands.
std::vector<Alias*> AliasTokens::extractAliasToken(llvm::AllocaInst* Inst) {
    // The operands are returned in the same order as they are present in the
    // instruction example x = alloca op1
    std::vector<Alias*> AliasVec;
    Alias* Alloca = this->getAliasToken(Inst);
    AliasVec.push_back(Alloca);
    AliasVec.push_back(this->getAliasToken(Alloca->getName().str() + "-orig",
                                           Inst->getParent()->getParent()));
    return AliasVec;
}

/// extractAliasToken - Returns a vector of alias objects for ReturnInst \Inst
/// operands.
std::vector<Alias*> AliasTokens::extractAliasToken(llvm::ReturnInst* Inst) {
    // The operands are returned in the same order as they are present in the
    // instruction example return op1
    llvm::Value* RetVal = Inst->getReturnValue();
    if (!llvm::isa<llvm::ConstantInt>(RetVal))
        return {this->getAliasToken(RetVal)};
}

/// extractAliasToken - Returns a vector of alias objects for BitCastInst \Inst
/// operands.
std::vector<Alias*> AliasTokens::extractAliasToken(llvm::BitCastInst* Inst) {
    // The operands are returned in the same order as they are present in the
    // instruction example x = bitcast op1
    std::vector<Alias*> AliasVec;
    AliasVec.push_back(this->getAliasToken(Inst));
    llvm::Instruction* PrevInst = Inst->getPrevNonDebugInstruction();
    if (llvm::CallInst* CI = llvm::dyn_cast<llvm::CallInst>(PrevInst)) {
        if (CI->getCalledFunction()->getName().startswith("_Zn")) {
            llvm::Instruction* NextInst = Inst->getNextNonDebugInstruction();
            if (llvm::CallInst* NextCI =
                    llvm::dyn_cast<llvm::CallInst>(NextInst)) {
                if (NextCI->getCalledFunction()->getName().startswith("_ZN")) {
                    AliasVec.push_back(this->getAliasToken(Inst->getDestTy()));
                }
            }
        } else if (CI->getCalledFunction()->getName().startswith("_ZN")) {
            llvm::Instruction* PrevInst = CI->getPrevNonDebugInstruction();
            if (llvm::BitCastInst* BI =
                    llvm::dyn_cast<llvm::BitCastInst>(PrevInst)) {
                llvm::Instruction* PrevInst = BI->getPrevNonDebugInstruction();
                if (llvm::CallInst* CI =
                        llvm::dyn_cast<llvm::CallInst>(PrevInst)) {
                    if (CI->getCalledFunction()->getName().startswith("_Zn")) {
                        AliasVec.push_back(
                            this->getAliasToken(Inst->getSrcTy()));
                    }
                }
            }
        }
    }
    if (AliasVec.size() == 1) {
        AliasVec.push_back(this->getAliasToken(Inst->getOperand(0)));
    }
    return AliasVec;
}

AliasTokens::~AliasTokens() {
    for (auto X : AliasBank) {
        delete X.second;
    }
}

}  // namespace AliasUtil
