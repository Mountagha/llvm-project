//===-- Cpu0.td - Describe the Cpu0 Target Machine ---------*- tablegen -*-===//

def FeatureCmp : SubtargetFeature<"cmp", "HasCmp", "true",
                                  "Enable 'cmp' instructions.">;

def FeatureSlt : SubtargetFeature<"slt", "HasSlt", "true",
                                  "Enable 'slt' instructions.">;

def FeatureCpu032I : SubtargetFeature<"cpu032I", "Cpu0ArchVersion",
                                      "Cpu032I", "Cpu032I ISA Support",
                                      [FeatureCmp]>;

def FeatureCpu032II : SubtargetFeature<"cpu032II", "Cpu0ArchVersion",
                                       "Cpu032II", "Cpu032II ISA Support",
                                       [FeatureCmp, FeatureSlt]>;

include "Cpu0Schedule.td"
include "Cpu0InstrInfo.td"
include "Cpu0CallingConv.td"

defm : RemapAllTargetPseudoPointerOperands<GPROut>;

def Cpu0InstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
    : Processor<Name, Cpu0GenericItineraries, Features>;

def : Proc<"cpu032I",  [FeatureCpu032I]>;
def : Proc<"cpu032II", [FeatureCpu032II]>;

// Will generate Cpu0GenAsmWrite.inc included by Cpu0InstPrinter.cpp, contents
//  as follows,
// void Cpu0InstPrinter::printInstruction(const MCInst *MI, raw_ostream &O) {...}
// const char *Cpu0InstPrinter::getRegisterName(unsigned RegNo) {...}
def Cpu0 : Target {
  // Def Cpu0InstrInfo : InstrInfo as before.
  let InstructionSet = Cpu0InstrInfo;
}