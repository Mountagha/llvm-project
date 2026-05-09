//===-- Cpu0MCAsmInfo.h - Cpu0 Asm Info ------------------------*- C++ -*-===//
//
// This file contains the declaration of the Cpu0MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_MCTARGETDESC_CPU0MCASMINFO_H
#define LLVM_LIB_TARGET_CPU0_MCTARGETDESC_CPU0MCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;
class MCTargetOptions;

class Cpu0MCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  Cpu0MCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
};

} // namespace llvm

#endif