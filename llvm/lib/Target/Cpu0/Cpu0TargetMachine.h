//===-- Cpu0TargetMachine.h - Define TargetMachine for Cpu0 -----*- C++ -*-===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0TARGETMACHINE_H
#define LLVM_LIB_TARGET_CPU0_CPU0TARGETMACHINE_H

#include <optional>

#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class Cpu0TargetMachine : public CodeGenTargetMachineImpl {
public:
    Cpu0TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                                        StringRef FS, const TargetOptions &Options,
                                        std::optional<Reloc::Model> RM,
                                        std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                                        bool JIT);
};

} // end namespace llvm

#endif