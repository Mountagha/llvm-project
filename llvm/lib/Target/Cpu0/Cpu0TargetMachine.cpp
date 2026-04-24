//===-- Cpu0TargetMachine.cpp - Define TargetMachine for Cpu0 -------------===//

#include "Cpu0TargetMachine.h"
#include "Cpu0.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "cpu0"

static std::string computeDataLayout(const Triple &TT) {
  // Keep this simple for the first skeleton.
  // Cpu0 is 32-bit. Endianness will eventually depend on cpu0/cpu0el.
  if (TT.isLittleEndian())
    return "e-m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32-S64";

  return "E-m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32-S64";
}

Cpu0TargetMachine::Cpu0TargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL, bool /*JIT*/)
    : CodeGenTargetMachineImpl(T, computeDataLayout(TT), TT, CPU, FS, Options,
                              RM.value_or(Reloc::Static),
                              CM.value_or(CodeModel::Small), OL) {}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0Target() {
  RegisterTargetMachine<Cpu0TargetMachine> X(TheCpu0Target);
  RegisterTargetMachine<Cpu0TargetMachine> Y(TheCpu0elTarget);
}