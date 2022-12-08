#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/DivergenceAnalysis.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include <fstream>
#include <filesystem>

using namespace llvm;
namespace fs = std::filesystem;

namespace Parser
{
    struct FeaturePass : public LoopPass
    {
        static char ID;
        FeaturePass() : LoopPass(ID) {}

        bool runOnLoop(Loop *L, LPPassManager &LPM) override
        {
            bool Changed = false;

            BranchProbabilityInfo &bpi = getAnalysis<BranchProbabilityInfoWrapperPass>().getBPI();
            // BlockFrequencyInfo &bfi = getAnalysis<BlockFrequencyInfoWrapperPass>().getBFI();
            LoopInfo &li = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
            // DivergenceInfo &di = new DivergenceInfo();

            // BasicBlock *Preheader = L->getLoopPreheader();
            // BasicBlock *Header = L->getHeader();

            auto countBB = 0;
            auto countBranches = 0;
            auto countTotInsts = 0;
            auto countFPInsts = 0;
            auto countIntInsts = 0;
            auto countLoads = 0;
            auto countStores = 0;

            for (Loop::block_iterator bb = L->block_begin(), e = L->block_end(); bb != e; ++bb)
            {
                BasicBlock *curr_bb = *bb;
                countBB += 1;
                for (BasicBlock::iterator i = curr_bb->begin(), e = curr_bb->end(); i != e; i++)
                {
                    countTotInsts += 1;
                    switch (i->getOpcode())
                    {
                    // Branch: br, switch, indirectbr
                    case Instruction::Br:
                    case Instruction::Switch:
                    case Instruction::IndirectBr:
                        countBranches += 1;
                        break;
                    // Integer ALU: add, sub, mul, udiv, sdiv, urem, shl, lshr, ashr, and,
                    // or, xor, icmp, srem
                    case Instruction::Add:
                    case Instruction::Sub:
                    case Instruction::Mul:
                    case Instruction::UDiv:
                    case Instruction::SDiv:
                    case Instruction::URem:
                    case Instruction::Shl:
                    case Instruction::LShr:
                    case Instruction::AShr:
                    case Instruction::And:
                    case Instruction::Or:
                    case Instruction::Xor:
                    case Instruction::ICmp:
                    case Instruction::SRem:
                        countIntInsts += 1;
                        break;
                    // Floating-point ALU: fadd, fsub, fmul, fdiv, frem, fcmp
                    case Instruction::FAdd:
                    case Instruction::FSub:
                    case Instruction::FMul:
                    case Instruction::FDiv:
                    case Instruction::FRem:
                    case Instruction::FCmp:
                        countFPInsts += 1;
                        break;
                    case Instruction::Load:
                        countLoads += 1;
                        break;
                    case Instruction::Store:
                        countStores += 1;
                        break;
                    default:
                        break;
                    }
                }
            }

            StringRef funcName = L->getHeader()->getParent()->getName();
            StringRef fileString = L->getHeader()->getParent()->getParent()->getSourceFileName();
            fs::path filePath = (std::string)fileString;
            std::string fileName = filePath.filename();
            
            if (funcName != "main") {
                errs() << "Number of Basic Blocks: " << countBB << "\n";
                errs() << "Number of Branches: " << countBranches << "\n";
                errs() << "Number of Total Instructions: " << countTotInsts << "\n";
                errs() << "Number of Integer Instructions: " << countIntInsts << "\n";
                errs() << "Number of Float Instructions: " << countFPInsts << "\n";
                errs() << "Number of Loads: " << countLoads << "\n";
                errs() << "Number of Stores: " << countStores << "\n";

                // Output/append to .csv file
                std::ofstream outfile;
                outfile.open("features.csv", std::ofstream::out | std::ofstream::app);
                if (!outfile) {
                    errs() << "File creation/append failed!" << "\n";
                } else {
                    outfile << fileName << "," << countBB << "," << countBranches << "," << countTotInsts << "," 
                        << countIntInsts << "," << countFPInsts << "," << countLoads << "," << countStores << std::endl;
                }
            }

            return Changed;
        }
        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.addRequired<BranchProbabilityInfoWrapperPass>();
            AU.addRequired<BlockFrequencyInfoWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();
        }

    private:
        bool inSubLoop(BasicBlock *BB, Loop *CurLoop, LoopInfo *LI)
        {
            assert(CurLoop->contains(BB) && "Only valid if BB is IN the loop");
            return LI->getLoopFor(BB) != CurLoop;
        }
    };
}

char Parser::FeaturePass::ID = 0;
static RegisterPass<Parser::FeaturePass>
    X("feature-parser",
      "Parse static features in a loop", false, false);