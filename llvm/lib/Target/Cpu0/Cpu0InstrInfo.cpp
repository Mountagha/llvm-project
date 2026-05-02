#include "Cpu0InstrInfo.h"

#include "Cpu0Subtarget.h"
#include "llvm/CodeGen/MachineInstr.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "Cpu0GenInstrInfo.inc"

// Pin the vtable to this file.
void Cpu0InstrInfo::anchor() {}

Cpu0InstrInfo::Cpu0InstrInfo(const Cpu0Subtarget &STI)
    : Cpu0GenInstrInfo(), Subtarget(STI) {}

const Cpu0InstrInfo *Cpu0InstrInfo::create(Cpu0Subtarget &STI) {
  return llvm::createCpu0SEInstrInfo(STI);
}

unsigned Cpu0InstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  return MI.getDesc().getSize();
}