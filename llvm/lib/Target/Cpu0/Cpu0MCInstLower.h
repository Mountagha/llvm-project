//===-- Cpu0MCInstLower.h - Lower MachineInstr to MCInst -------*- C++ -*--===//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0MCINSTLOWER_H
#define LLVM_LIB_TARGET_CPU0_CPU0MCINSTLOWER_H


#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {

    class MCContext;
    class MCInst;
    class MCOperand;
    class MachineInstr;
    class MachineFunction;
    class Cpu0AsmPrinter;

    /// This class is used to lower a MachineInstr into an MCInst.
    class LLVM_LIBRARY_VISIBILITY Cpu0MCInstLower {
        using MachineOperandType = MachineOperand::MachineOperandType;

        MCContext *Ctx;
        Cpu0AsmPrinter &AsmPrinter;
    
    public:
        Cpu0MCInstLower(Cpu0AsmPrinter &asmprinter);
        void Initialize(MCContext* C);
        void Lower(const MachineInstr* MI, MCInst &OutMI) const;
        MCOperand LowerOperand(const MachineOperand &MO, unsigned offset = 0) const;
    };
} // end namespace llvm

#endif