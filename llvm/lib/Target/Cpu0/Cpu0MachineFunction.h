//===-- Cpu0MachineFunctionInfo.h - Private data used for Cpu0 ----*- C++ -*-=//
//===----------------------------------------------------------------------===//
//
// This file declares the Cpu0 specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0MACHINEFUNCTION_H
#define LLVM_LIB_TARGET_CPU0_CPU0MACHINEFUNCTION_H

#include "Cpu0Config.h"

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
    : MF(MF),
    varArgsFrameIndex(0),
    MaxCallFrameSize(0)
    {}

    ~Cpu0FunctionInfo();

    int getVarArgsFrameIndex() const { return varArgsFrameIndex; }
    void setVargArgsFrameIndex(int Index) { varArgsFrameIndex = Index; }
private:
    virtual void anchor();

    MachineFunction &MF;

    /// VarArgsFrameIndex - FrameIndex for start of varargs area.
    int VarArgsFrameIndex;

    unsigned MaxCallFrameSize;
};

}   // end of namespace llvm

#endif // CPU0_MACHINE_FUNCTION_INFO_H