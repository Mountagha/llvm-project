#ifndef LLVM_LIB_TARGET_CPU0_CPU0TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_CPU0_CPU0TARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class Cpu0TargetMachine;
class MCContext;
class MCSection;
class TargetMachine;

class Cpu0TargetObjectFile : public TargetLoweringObjectFileELF {
    MCSection *SmallDataSection = nullptr;
    MCSection *SmallBSSSection = nullptr;
    const Cpu0TargetMachine *TM = nullptr;

public:
    void Initialize(MCContext &Ctx, const TargetMachine &TM) override;
};

}   // end namespace llvm

#endif