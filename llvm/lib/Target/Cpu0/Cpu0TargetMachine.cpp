//===-- Cpu0TargetMachine.cpp - Define TargetMachine for Cpu0 -------------===//

#include "Cpu0TargetMachine.h"
#include "Cpu0.h"
#include "Cpu0TargetObjectFile.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "cpu0"

static std::string computeDataLayout(const Triple &TT, StringRef CPU,
                                     const TargetOptions &Options,
                                     bool IsLittle) {
  
  std::string Ret;

  Ret += IsLittle ? "e" : "E";

  // ELF mangling.
  Ret += "-m:e";

  // Cpu0 is 32-bit.
  Ret += "-p:32:32";

  // Match the tutorial's ABI assumptions.
  Ret += "-i8:8:32-i16:16:32-i64:64";

  // Native integer with: 32 bits. Statck alignment : 64 bits
  Ret += "-n32-S64";

  return Ret;
}

static Reloc::Model getEffectiveRelocModel(bool JIT,
                                           std::optional<Reloc::Model> RM) {
  if (!RM || JIT)
    return Reloc::Static;
  return *RM;
}

// DataLayout --> Big-endian, 32-bit pointer/ABI/alignment
// The stack is always 8 byte aligned
// On function prologue, the stack is created by decrementing
// its pointer. Once decremented, all references are done with positive
// offset from the stack/frame pointer, using StackGrowsUp enables
// an easier handling.
// Using CodeModel::Large enables different CALL behavior.
Cpu0TargetMachine::Cpu0TargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL, bool JIT,
    bool IsLittle)
    : CodeGenTargetMachineImpl(
          T, computeDataLayout(TT, CPU, Options, IsLittle), TT, CPU, FS,
          Options, getEffectiveRelocModel(JIT, RM),
          getEffectiveCodeModel(CM, CodeModel::Small), OL),
      IsLittle(IsLittle), TLOF(std::make_unique<Cpu0TargetObjectFile>()),
      ABI(Cpu0ABIInfo::computeTargetABI()),
      DefaultSubtarget(TT, CPU, FS, IsLittle, *this) {
  initAsmInfo();
}

Cpu0TargetMachine::~Cpu0TargetMachine() = default;

void Cpu0ebTargetMachine::anchor() {}

Cpu0ebTargetMachine::Cpu0ebTargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL, bool JIT)
    : Cpu0TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT,
                        /*IsLittle=*/false) {}

void Cpu0elTargetMachine::anchor() {}

Cpu0elTargetMachine::Cpu0elTargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CM, CodeGenOptLevel OL, bool JIT)
    : Cpu0TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT,
                        /*IsLittle=*/true) {}

const Cpu0Subtarget *
Cpu0TargetMachine::getSubtargetImpl(const Function &F) const {
  std::string CPU = std::string(getTargetCPU());
  std::string FS = std::string(getTargetFeatureString());

  // Later you can include function attributes in this key if Cpu0 supports
  // per-function target-cpu/target-features.
  std::string Key = CPU + FS;

  auto &I = SubtargetMap[Key];

  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<Cpu0Subtarget>(getTargetTriple(), CPU, FS, IsLittle,
                                        *this);
  }

  return I.get();
}

TargetPassConfig *
Cpu0TargetMachine::createPassConfig(legacy::PassManagerBase &PM) {
  return new TargetPassConfig(*this, PM);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0Target() {
  RegisterTargetMachine<Cpu0ebTargetMachine> X(TheCpu0Target);
  RegisterTargetMachine<Cpu0elTargetMachine> Y(TheCpu0elTarget);
}