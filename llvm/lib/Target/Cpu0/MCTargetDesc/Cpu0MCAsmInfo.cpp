//===-- Cpu0MCAsmInfo.cpp - Cpu0 Asm Properties --------------------------===//
//
// This file contains the declarations of the Cpu0MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "Cpu0MCAsmInfo.h"

#include "llvm/TargetParser/Triple.h"

using namespace llvm;

void Cpu0MCAsmInfo::anchor() {}

Cpu0MCAsmInfo::Cpu0MCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
  CodePointerSize = CalleeSaveStackSlotSize = 4;
  IsLittleEndian = TT.isLittleEndian();
  setTargetOptions(Options);

  AlignmentIsInBytes = false;
  Data16bitsDirective = "\t.2byte\t";
  Data32bitsDirective = "\t.4byte\t";
  Data64bitsDirective = "\t.8byte\t";
  // PrivateLabelPrefix: display $BB for the labels of basic block.
  PrivateLabelPrefix = "$";
  CommentString = "#";
  ZeroDirective = "\t.space\t";
  WeakRefDirective = "\t.weak\t";
  UseAssignmentForEHBegin = true;

  SupportsDebugInformation = true;
  DwarfRegNumForCFI = true;
}