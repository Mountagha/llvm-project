//===-- Cpu0TargetMachine.h - Define TargetMachine for Cpu0 -----*- C++ -*-===//

#ifndef LLVM_LIB_TARGET_CPU0_CPU0TARGETMACHINE_H
#define LLVM_LIB_TARGET_CPU0_CPU0TARGETMACHINE_H

#include <optional>
#include <memory>

#include "Cpu0Subtarget.h"
#include "MCTargetDesc/Cpu0ABIInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class Cpu0RegisterInfo;
class Function;
class TargetLoweringObjectFile;

class Cpu0TargetMachine : public CodeGenTargetMachineImpl {
    bool IsLittle;
    std::unique_ptr<TargetLoweringObjectFile> TLOF;

    Cpu0ABIInfo ABI;
    Cpu0Subtarget DefaultSubtarget;

    mutable StringMap<std::unique_ptr<Cpu0Subtarget>> SubtargetMap;

public:
    Cpu0TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                                        StringRef FS, const TargetOptions &Options,
                                        std::optional<Reloc::Model> RM,
                                        std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                                        bool JIT, bool IsLittle);

    ~Cpu0TargetMachine() override;

    const Cpu0Subtarget *getSubtargetImpl() const {
        return &DefaultSubtarget;
    }

    const Cpu0Subtarget *getSubtargetImpl(const Function &F) const override;

    TargetPassConfig *createPassConfig(legacy::PassManagerBase &PM) override;

    TargetLoweringObjectFile *getObjFileLowering() const override {
        return TLOF.get();
    }

    bool isLittleEndian() const { return IsLittle; }

    const Cpu0ABIInfo &getABI() const { return ABI; }
};

/// Cpu032 big endian target machine.
class Cpu0ebTargetMachine : public Cpu0TargetMachine {
    virtual void anchor();
public:
    Cpu0ebTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                                            StringRef FS, const TargetOptions &Options,
                                            std::optional<Reloc::Model> RM,
                                            std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                                            bool JIT);


};

/// Cpu032 little endian target machine.
class Cpu0elTargetMachine : public Cpu0TargetMachine {
    virtual void anchor();
public:
    Cpu0elTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                                            StringRef FS, const TargetOptions &Options,
                                            std::optional<Reloc::Model> RM,
                                            std::optional<CodeModel::Model> CM, CodeGenOptLevel OL,
                                            bool JIT);


};

} // end namespace llvm

#endif