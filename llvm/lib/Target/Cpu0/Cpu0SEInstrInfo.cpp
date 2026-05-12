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

/// Expand Pseudo instructions into real backend instructions
bool Cpu0SEInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();

  switch (MI.getDesc().getOpcode())
  {
  case Cpu0::RetLR:
    expandRetLR(MBB, MI);
    break;
  
  default:
    return false;
  }

  MBB.erase(MI);
  return true;
}

void Cpu0SEInstrInfo::expandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const {
  BuildMI(MBB, I, I->getDebugLoc(), get(Cpu0::RET)).addReg(Cpu0::LR);
}
