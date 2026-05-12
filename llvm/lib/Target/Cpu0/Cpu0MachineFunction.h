//===-- Cpu0MachineFunctionInfo.h - Private data used for Cpu0 ----*- C++ -*-=//
//===----------------------------------------------------------------------===//
//
// This file declares the Cpu0 specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0MACHINEFUNCTION_H
#define LLVM_LIB_TARGET_CPU0_CPU0MACHINEFUNCTION_H

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/Target/TargetMachine.h"
#include <map>

namespace llvm {

/// Cpu0FunctionInfo - this class is derived from MachineFunction private
/// Cpu0 target-specific information for each MachineFunction.
class Cpu0FunctionInfo : public MachineFunctionInfo {
public:
    Cpu0FunctionInfo(MachineFunction &MF)
    : SRetReturnReg(0),
    HasByvalArg(false),
    IncomingArgSize(0),
    CallsEhReturn(false),
    CallsEhDwarf(false),
    MF(MF),
    VarArgsFrameIndex(0),
    MaxCallFrameSize(0),
    EmitNOAT(false)
    {}

    ~Cpu0FunctionInfo();

    int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
    void setVargArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
    bool getEmitNOAT() const { return EmitNOAT; }
    void setEmitNOAT() { EmitNOAT = true; }

    unsigned getSRetReturnReg() const { return SRetReturnReg; }
    void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }

    bool hasByvalArg() const { return HasByvalArg; }
    void setFormalArgInfo(unsigned Size, bool HasByval) {
        IncomingArgSize = Size;
        HasByvalArg = HasByval;
    }

private:
    virtual void anchor();

    /// SRetReturnReg - Some subtargets require that sret lowering includes
    /// returning the value of the returned struct in a register. This field
    /// holds the virtual register into which the sret argument is passed.
    unsigned SRetReturnReg;

    /// True if function has a byval argument.
    bool HasByvalArg;

    /// Size of incoming argument area.
    unsigned IncomingArgSize;

    /// CallsEhReturn - Whether the function calls llvm.eh.return.
    bool CallsEhReturn;

    /// CallsEhDwarf - Whether the function calls llvm.eh.dwarf.
    bool CallsEhDwarf;

    /// Frame objects for spilling eh data registers.
    int EhDataRegFI[2];    


    MachineFunction &MF;

    /// VarArgsFrameIndex - FrameIndex for start of varargs area.
    int VarArgsFrameIndex;

    unsigned MaxCallFrameSize;
    bool EmitNOAT;
};

}   // end of namespace llvm

#endif // CPU0_MACHINE_FUNCTION_INFO_H