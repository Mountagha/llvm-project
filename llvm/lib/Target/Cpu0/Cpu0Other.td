//===-- Cpu0Other.td - Describe the Cpu0 Target Machine ----*- tablegen -*-===//

//===----------------------------------------------------------------------===//
// This is the top level entry point for the Cpu0 target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Target-dependent interfaces
//===----------------------------------------------------------------------===//

include "Cpu0RegisterInfo.td"
include "Cpu0RegisterInfoGPROutForOther.td"
include "Cpu0.td"