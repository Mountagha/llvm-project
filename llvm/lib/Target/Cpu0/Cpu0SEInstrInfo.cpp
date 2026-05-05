//===-- Cpu0SEInstrInfo.cpp - Cpu032 Instruction Information -------------===//
//
//===----------------------------------------------------------------------===//

#include "Cpu0SEInstrInfo.h"

using namespace llvm;

Cpu0SEInstrInfo::Cpu0SEInstrInfo(const Cpu0Subtarget &STI)
    : Cpu0InstrInfo(STI, RI, Cpu0::JR), RI(STI) {}

const Cpu0InstrInfo *llvm::createCpu0SEInstrInfo(const Cpu0Subtarget &STI) {
  return new Cpu0SEInstrInfo(STI);
}
