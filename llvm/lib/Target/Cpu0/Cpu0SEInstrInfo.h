//===-- Cpu0SEInstrInfo.h - Cpu032 Instruction Information -----*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains the Cpu032 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0SEINSTRINFO_H
#define LLVM_LIB_TARGET_CPU0_CPU0SEINSTRINFO_H

#include "Cpu0InstrInfo.h"
#include "Cpu0SERegisterInfo.h"

namespace llvm {

class Cpu0SEInstrInfo : public Cpu0InstrInfo {
  const Cpu0SERegisterInfo RI;

public:
  explicit Cpu0SEInstrInfo(const Cpu0Subtarget &STI);

  const Cpu0SERegisterInfo &getRegisterInfo() const override { return RI; }
};

} // end namespace llvm

#endif