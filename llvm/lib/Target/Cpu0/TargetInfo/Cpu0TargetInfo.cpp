//===-- Cpu0TargetInfo.cpp - Cpu0 Target Implementation -------------------===//
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/Triple.h"
#include "Cpu0.h"

using namespace llvm;

namespace llvm {
Target TheCpu0Target;
Target TheCpu0elTarget;
} // namespace llvm

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0TargetInfo() {
  RegisterTarget<Triple::cpu0, true> X(llvm::TheCpu0Target, "cpu0",
                                        "CPU0 (32-bit big endian)", "Cpu0");
  RegisterTarget<Triple::cpu0el, true> Y(llvm::TheCpu0elTarget, "cpu0el",
                                          "CPU0 (32-bit little endian)", "Cpu0");
}