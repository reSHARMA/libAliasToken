#include "AliasToken/AliasToken.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
using namespace llvm;
using namespace AliasUtil;

namespace {

#define DEBUG_TYPE "test"

class TestPass : public ModulePass {
   public:
    static char ID;
    TestPass() : ModulePass(ID) {}

    bool runOnModule(Module& M) override {
        bool converged = false;
        AliasTokens AT;
        for (Function& F : M.functions()) {
            for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E;
                 ++I) {
                if (StoreInst* SI = dyn_cast<StoreInst>(&*I)) {
                    auto AliasVec = AT.extractAliasToken(SI);
                    if (!isa<ConstantInt>(SI->getValueOperand()))
                        assert(AliasVec.size() == 2 &&
                               "StoreInst should return two alias token unless "
                               "the value is const");
                    else
                        assert(AliasVec.size() == 1 &&
                               "StoreInst should return one alias token if the "
                               "value is const");
                }
            }
        }
        return false;
    }
};
}  // namespace

char TestPass::ID = 0;
static RegisterPass<TestPass> X("test", "Simple pass for testing", true, true);
