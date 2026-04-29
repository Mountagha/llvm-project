//===-- Cpu0MCTargetDesc.cpp - Cpu0 Target Descriptions -------------------===//
//
// This file provides Cpu0 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "Cpu0MCTargetDesc.h"
#include "llvm/MC/MCAsmInfoELF.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"

using namespace llvm;

namespace {

class Cpu0MCAsmInfo : public MCAsmInfoELF {
public:
  Cpu0MCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
    CodePointerSize = CalleeSaveStackSlotSize = 4;
    IsLittleEndian = TT.isLittleEndian();
    setTargetOptions(Options);
  }
};

} // end anonymous namespace

#define GET_INSTRINFO_MC_DESC
#include "Cpu0GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Cpu0GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "Cpu0GenRegisterInfo.inc"

namespace {

static MCAsmInfo *createCpu0MCAsmInfo(const MCRegisterInfo &MRI,
                                      const Triple &TT,
                                      const MCTargetOptions &Options) {
  return new Cpu0MCAsmInfo(TT, Options);
}

static MCInstrInfo *createCpu0MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitCpu0MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createCpu0MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitCpu0MCRegisterInfo(X, Cpu0::LR);
  return X;
}

static MCSubtargetInfo *createCpu0MCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU,
                                                  StringRef FS) {
  if (CPU.empty())
    CPU = "cpu032I";
  return createCpu0MCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

static void registerCpu0MCComponents(Target &T) {
  TargetRegistry::RegisterMCAsmInfo(T, createCpu0MCAsmInfo);
  TargetRegistry::RegisterMCInstrInfo(T, createCpu0MCInstrInfo);
  TargetRegistry::RegisterMCRegInfo(T, createCpu0MCRegisterInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createCpu0MCSubtargetInfo);
}

} // end anonymous namespace

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeCpu0TargetMC() {
  for (const Target &T : TargetRegistry::targets()) {
    if (StringRef(T.getName()) == "cpu0" || StringRef(T.getName()) == "cpu0el") {
      registerCpu0MCComponents(const_cast<Target &>(T));
    }
  }
}