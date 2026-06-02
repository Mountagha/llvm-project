//===-- Cpu0ELFObjectWriter.cpp - Cpu0 ELF Writer -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Cpu0BaseInfo.h"
#include "MCTargetDesc/Cpu0FixupKinds.h"
#include "MCTargetDesc/Cpu0MCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <list>

using namespace llvm;

// Cpu0 ELF Relocation Types (custom values for this architecture)
namespace {
// Machine type for Cpu0.
const unsigned EM_CPU0 = 0xFD;

// Cpu0 ELF relocation types.
const unsigned R_CPU0_NONE = 0;
const unsigned R_CPU0_32 = 1;
const unsigned R_CPU0_GPREL16 = 3;
const unsigned R_CPU0_GOT16 = 5;
const unsigned R_CPU0_HI16 = 7;
const unsigned R_CPU0_LO16 = 8;
const unsigned R_CPU0_GOT_HI16 = 13;
const unsigned R_CPU0_GOT_LO16 = 14;

class Cpu0ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  Cpu0ELFObjectWriter(uint8_t OSABI, bool HasRelocationAddend, bool Is64);
  ~Cpu0ELFObjectWriter() override = default;

  unsigned getRelocType(const MCFixup &Fixup, const MCValue &Target,
                        bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCValue &Sym,
                               unsigned Type) const override;
};
} // namespace

Cpu0ELFObjectWriter::Cpu0ELFObjectWriter(uint8_t OSABI,
                                         bool HasRelocationAddend, bool Is64)
    : MCELFObjectTargetWriter(/*Is64Bit_=false*/ Is64, OSABI, EM_CPU0,
          /*HasRelocationAddend_ = false*/ HasRelocationAddend) {}

//@GetRelocType {
unsigned Cpu0ELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                           const MCValue &Target,
                                           bool IsPCRel) const {
  // determine the type of the relocation
  unsigned Type = (unsigned)R_CPU0_NONE;
  unsigned Kind = (unsigned)Fixup.getKind();

  switch (Kind) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case FK_Data_4:
    Type = R_CPU0_32;
    break;
  case Cpu0::fixup_Cpu0_32:
    Type = R_CPU0_32;
    break;
  case Cpu0::fixup_Cpu0_GPREL16:
    Type = R_CPU0_GPREL16;
    break;
  case Cpu0::fixup_Cpu0_GOT:
    Type = R_CPU0_GOT16;
    break;
  case Cpu0::fixup_Cpu0_HI16:
    Type = R_CPU0_HI16;
    break;
  case Cpu0::fixup_Cpu0_LO16:
    Type = R_CPU0_LO16;
    break;
  case Cpu0::fixup_Cpu0_GOT_HI16:
    Type = R_CPU0_GOT_HI16;
    break;
  case Cpu0::fixup_Cpu0_GOT_LO16:
    Type = R_CPU0_GOT_LO16;
    break;
  }

  return Type;
}
//@GetRelocType }

bool Cpu0ELFObjectWriter::needsRelocateWithSymbol(const MCValue &Sym,
                                                  unsigned Type) const {
  // FIXME: This is extremelly conservative. This really needs to use a
  // whitelist with a clear explanation for why each realocation needs to
  // point to the symbol, not to the section.
  switch (Type) {
  default:
    return true;

  case R_CPU0_GOT16:
  // For Cpu0 pic mode, I think it's OK to return true but I didn't confirm.
  //  llvm_unreachable("Should have been handled already");
    return true;

  // These relocations might be paired with another relocation. The pairing is
  // done by the static linker by matching the symbol. Since we only see one
  // relocation at a time, we have to force them to relocate with a symbol to
  // avoid ending up with a pair where one points to a section and another
  // points to a symbol.
  case R_CPU0_HI16:
  case R_CPU0_LO16:
  // R_CPU0_32 should be a relocation record, I don't know why Mips set it to 
  // false.
  case R_CPU0_32:
    return true;

  case R_CPU0_GPREL16:
    return false;
  }
}

std::unique_ptr<MCObjectTargetWriter> 
llvm::createCpu0ELFObjectWriter(const Triple &TT) {
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
  bool IsN64 = false;
  bool HasRelocationAddend = TT.isArch64Bit();
  return std::make_unique<Cpu0ELFObjectWriter>(OSABI, HasRelocationAddend,
                                               IsN64);
}