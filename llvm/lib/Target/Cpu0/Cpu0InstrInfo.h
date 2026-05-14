#ifndef LLVM_LIB_TARGET_CPU0_CPU0INSTRINFO_H
#define LLVM_LIB_TARGET_CPU0_CPU0INSTRINFO_H

#include "Cpu0.h"
#include "Cpu0RegisterInfo.h"
#include "Cpu0AnalyzeImmediate.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "Cpu0GenInstrInfo.inc"

namespace llvm {

class Cpu0Subtarget;

class Cpu0InstrInfo : public Cpu0GenInstrInfo {
    virtual void anchor();

protected:
    const Cpu0Subtarget &Subtarget;

public:
    Cpu0InstrInfo(const Cpu0Subtarget &STI, const Cpu0RegisterInfo &RI,
                  unsigned ReturnOpcode);

    static const Cpu0InstrInfo *create(Cpu0Subtarget &STI);

    /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
    /// such, whenever a client has an instance of instruction info, it should
    /// always be able to get register info as well (through this method).
    ///
    virtual const Cpu0RegisterInfo &getRegisterInfo() const = 0;

    /// Return the number of bytes of code the specified instruction may be.
    unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

    void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MBBI,
                           Register SrcReg, bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                                                     Register VReg,
                                                     MachineInstr::MIFlag Flags =
                                                             MachineInstr::NoFlags) const override {
            storeRegToStack(MBB, MBBI, SrcReg, isKill, FrameIndex, RC, 0, Flags);
    }

    void loadRegFromStackSlot(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MBBI,
                              Register DestReg, int FrameIndex,
                              const TargetRegisterClass *RC,
                                                            Register VReg, unsigned SubReg = 0,
                                                            MachineInstr::MIFlag Flags =
                                                                    MachineInstr::NoFlags) const override {
            loadRegFromStack(MBB, MBBI, DestReg, FrameIndex, RC, 0, Flags);
    }

    virtual void storeRegToStack(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                Register SrcReg, bool isKill, int FrameIndex,
                                const TargetRegisterClass *RC,
                                                                int64_t Offset,
                                                                MachineInstr::MIFlag Flags =
                                                                        MachineInstr::NoFlags) const = 0;

    virtual void loadRegFromStack(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MI,
                                  Register DestReg, int FrameIndex,
                                  const TargetRegisterClass *RC,
                                                                    int64_t Offset,
                                                                    MachineInstr::MIFlag Flags =
                                                                            MachineInstr::NoFlags) const = 0;

        MachineMemOperand *GetMemOperand(MachineBasicBlock &MBB, int FI,
                                                                    MachineMemOperand::Flags Flags) const;
        };

const Cpu0InstrInfo *createCpu0SEInstrInfo(const Cpu0Subtarget &STI);

} // end namespace llvm

#endif