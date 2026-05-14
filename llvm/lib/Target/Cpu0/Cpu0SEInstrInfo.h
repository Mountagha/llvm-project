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
#include "Cpu0AnalyzeImmediate.h"

namespace llvm {

class Cpu0SEInstrInfo : public Cpu0InstrInfo {
  const Cpu0SERegisterInfo RI;

public:
  explicit Cpu0SEInstrInfo(const Cpu0Subtarget &STI);

  const Cpu0SERegisterInfo &getRegisterInfo() const override { return RI; }

  bool expandPostRAPseudo(MachineInstr &MI) const override;

  /// Adjust SP by Amount bytes.
  void adjustStackPtr(unsigned SP, int64_t Amount, MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator I) const;

  /// Emit a series of instructions to load an immediate. If NewImm is a
  /// non-NULL parameter, the last instruction is not emitted, but instead
  /// its immediate operand is returned in NewImm.
  unsigned loadImmediate(int64_t Imm, MachineBasicBlock &MBB,
                         MachineBasicBlock::iterator II, const DebugLoc &DL,
                         unsigned *NewImm) const;

  void storeRegToStack(MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator MI,
                      Register SrcReg, bool isKill, int FrameIndex,
                      const TargetRegisterClass *RC,
              int64_t Offset,
              MachineInstr::MIFlag Flags =
                MachineInstr::NoFlags) const override;

  void loadRegFromStack(MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator MI,
                      Register DestReg, int FrameIndex,
                      const TargetRegisterClass *RC,
              int64_t Offset,
              MachineInstr::MIFlag Flags =
                MachineInstr::NoFlags) const override;
  
  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                  const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                  bool KillSrc) const override;
private:
  void expandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const;

};

} // end namespace llvm

#endif