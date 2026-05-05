//===-- Cpu0ISelLowering.h - Cpu0 DAG Lowering Interface --------*- C++ -*-===//

//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Cpu0 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0ISELLOWERING_H
#define LLVM_LIB_TARGET_CPU0_CPU0ISELLOWERING_H

#include "MCTargetDesc/Cpu0ABIInfo.h"
#include "Cpu0.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/TargetLowering.h"
#include <deque>

namespace llvm {
    namespace Cpu0ISD {
        enum NodeType {
            // Start the numbering from where ISD NodeType finishes.
            FIRST_NUMBER = ISD::BUILTIN_OP_END,

            // Jump and link (call)
            JumpLink,

            // Tail call
            TailCall,

            // Get the Higher 16 bits from a 32-bit immediate
            // No relation with Cpu0 Hi register
            Hi,

            // Get the lower 16 bits from a 32-bit immediate
            // No relation with Cpu0 Lo register
            Lo,

            // Handle gp_rel (small data/bss sections) relocation.
            GPRel,

            // Thread Pointer
            ThreadPointer,

            // Return
            Ret,

            EH_RETURN,

            // DivRem(u)
            DivRem,
            DivRemU,

            Wrapper,
            DynAlloc,

            Sync

        };
    }


    //===--------------------------------------------------------------------===//
    // TargetLowering Implementation
    //===--------------------------------------------------------------------===//

    class Cpu0FunctionInfo;
    class Cpu0Subtarget;

    // @class Cpu0TargetLowering
    class Cpu0TargetLowering : public TargetLowering {
    public:
        explicit Cpu0TargetLowering(const Cpu0TargetMachine &TM,
                                    const Cpu0Subtarget &STI);
        static const Cpu0TargetLowering *create(const Cpu0TargetMachine &TM,
                                                const Cpu0Subtarget &STI);
        // getTargetNodeName - This method returns the name of a target specific
        // DAG node.
        const char *getTargetNodeName(unsigned Opcode) const override;

    protected:
        /// ByValArgInfo - Byval argument information.
        struct ByValArgInfo {
            unsigned FirstIdx;  // Index of the first register used.
            unsigned NumRegs;   // Number of registers used for this argument.
            unsigned Address;   // Offset of the stack area used to pass this argument.

            ByValArgInfo() : FirstIdx(0), NumRegs(0), Address(0) {}
        };
    
    protected:
        // SubTarget Info
        const Cpu0Subtarget &SubTarget;
        // Cache the ABI from the TargetMachine, we use it everywhere.
        const Cpu0ABIInfo &ABI;
    
    private:
        // Lower Operand specifics
        SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

        // - must exist even without function call.
        SDValue LowerFormalArguments(SDValue Chain,
                                    CallingConv::ID CallConv, bool isVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    const SDLoc &dl, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals) const override;
        
        SDValue LowerReturn(SDValue Chain,
                            CallingConv::ID CallConv, bool isVarArg,
                            const SmallVectorImpl<ISD::OutputArg> &Outs,
                            const SmallVectorImpl<SDValue> &OutVals,
                            const SDLoc &dl, SelectionDAG &DAG) const override;

    };
    const Cpu0TargetLowering *
    createCpu0SETargetLowering(const Cpu0TargetMachine &TM, const Cpu0Subtarget &STI);
} // end namespace llvm

#endif  // Cpu0ISELLOWERING_H