#include "Cpu0TargetObjectFile.h"

#include "Cpu0TargetMachine.h"
#include "llvm/MC/MCContext.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

static cl::opt<unsigned>
    SSThreshold("cpu0-ssection-threshold", cl::Hidden,
                cl::desc("Small data and bss section threshold size"),
                cl::init(8));

void Cpu0TargetObjectFile::Initialize(MCContext &Ctx,
                                    const TargetMachine &TM) {

    TargetLoweringObjectFileELF::Initialize(Ctx, TM);

    SmallDataSection = getContext().getELFSection(
        ".sdata", ELF::SHT_PROGBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);
    
    SmallBSSSection = getContext().getELFSection(
        ".sbss", ELF::SHT_NOBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);

    this->TM = &static_cast<const Cpu0TargetMachine &>(TM);
}

