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

void Cpu0SEInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I,
                                  const DebugLoc &DL, Register DestReg,
                                  Register SrcReg, bool KillSrc,
                                  bool RenamableDest,
                                  bool RenamableSrc) const {
  (void)RenamableDest;
  (void)RenamableSrc;
  unsigned Opc = 0, ZeroReg = 0;

  if (Cpu0::CPURegsRegClass.contains(DestReg)) { // Copy to CPU Reg.
    if (Cpu0::CPURegsRegClass.contains(SrcReg))
      Opc = Cpu0::ADDu, ZeroReg = Cpu0::ZERO;
    else if (SrcReg == Cpu0::HI)
      Opc = Cpu0::MFHI, SrcReg = 0;
    else if (SrcReg == Cpu0::LO)
      Opc = Cpu0::MFLO, SrcReg = 0;
  }
  else if (Cpu0::CPURegsRegClass.contains(SrcReg)) { // Copy from CPU Reg.
    if (DestReg == Cpu0::HI)
      Opc = Cpu0::MTHI, DestReg = 0;
    else if (DestReg == Cpu0::LO)
      Opc = Cpu0::MTLO, DestReg = 0;
  }

  assert(Opc && "Cannot copy registers");

  MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Opc));

  if (DestReg)
    MIB.addReg(DestReg, RegState::Define);

  if (ZeroReg)
    MIB.addReg(ZeroReg);

  if (SrcReg)
    MIB.addReg(SrcReg, getKillRegState(KillSrc));
}

void Cpu0SEInstrInfo::expandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const {
  BuildMI(MBB, I, I->getDebugLoc(), get(Cpu0::RET)).addReg(Cpu0::LR);
}

/// Adjust SP by Amount bytes.
void Cpu0SEInstrInfo::adjustStackPtr(unsigned SP, int64_t Amount,
                                    MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator I) const {
  DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();
  unsigned ADDu = Cpu0::ADDu;
  unsigned ADDiu = Cpu0::ADDiu;

  if (isInt<16>(Amount)) {
    // addiu sp, sp, amount
    BuildMI(MBB, I, DL, get(ADDiu), SP).addReg(SP).addImm(Amount);
  } else { // Expand immediate that does not fit in 16-bit.
    unsigned Reg = loadImmediate(Amount, MBB, I, DL, nullptr);
    BuildMI(MBB, I, DL, get(ADDu), SP)
        .addReg(SP)
        .addReg(Reg, RegState::Kill);
  }
}

/// This function generates the sequence of instructions needed to get the
/// result of adding register REG and immediate IMM.
unsigned
Cpu0SEInstrInfo::loadImmediate(int64_t Imm, MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator II,
                               const DebugLoc &DL,
                               unsigned *NewImm) const {
  Cpu0AnalyzeImmediate AnalyzeImm;
  unsigned Size = 32;
  unsigned LUi = Cpu0::LUi;
  unsigned ZEROReg = Cpu0::ZERO;
  unsigned ATReg = Cpu0::AT;
  bool LastInstrIsADDiu = NewImm;

  const Cpu0AnalyzeImmediate::InstSeq &Seq =
    AnalyzeImm.Analyze(Imm, Size, LastInstrIsADDiu);
  Cpu0AnalyzeImmediate::InstSeq::const_iterator Inst = Seq.begin();

  assert(Seq.size() && (!LastInstrIsADDiu || (Seq.size() > 1)));

  // The first instruction can be a LUi, which is different from other
  // instructions (ADDiu, ORI and SLL) in that it does not have a register
  // operand.
  if (Inst->Opc == LUi)
    BuildMI(MBB, II, DL, get(LUi), ATReg).addImm(SignExtend64<16>(Inst->ImmOpnd));
  else
    BuildMI(MBB, II, DL, get(Inst->Opc), ATReg).addReg(ZEROReg)
      .addImm(SignExtend64<16>(Inst->ImmOpnd));

  // Build the remaining instructions in Seq.
  for (++Inst; Inst != Seq.end() - LastInstrIsADDiu; ++Inst)
    BuildMI(MBB, II, DL, get(Inst->Opc), ATReg).addReg(ATReg)
      .addImm(SignExtend64<16>(Inst->ImmOpnd));

  if (LastInstrIsADDiu)
    *NewImm = Inst->ImmOpnd;

  return ATReg;
}

void Cpu0SEInstrInfo::
storeRegToStack(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                Register SrcReg, bool isKill, int FI,
                const TargetRegisterClass *RC, int64_t Offset,
                MachineInstr::MIFlag Flags) const {
  (void)RC;
  DebugLoc DL;
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);

  unsigned Opc = 0;

  Opc = Cpu0::ST;
  assert(Opc && "Register class not handled!");
  BuildMI(MBB, I, DL, get(Opc))
      .setMIFlag(Flags)
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FI)
      .addImm(Offset)
      .addMemOperand(MMO);
}

void Cpu0SEInstrInfo::
loadRegFromStack(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                 Register DestReg, int FI, const TargetRegisterClass *RC,
                 int64_t Offset, MachineInstr::MIFlag Flags) const {
  (void)RC;
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOLoad);
  unsigned Opc = 0;

  Opc = Cpu0::LD;
  assert(Opc && "Register class not handled!");
  BuildMI(MBB, I, DL, get(Opc), DestReg)
      .setMIFlag(Flags)
      .addFrameIndex(FI)
      .addImm(Offset)
      .addMemOperand(MMO);
}

