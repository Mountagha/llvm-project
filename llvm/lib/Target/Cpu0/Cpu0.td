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

defm : RemapAllTargetPseudoPointerOperands<GPROut>;

def Cpu0InstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
    : Processor<Name, Cpu0GenericItineraries, Features>;

def : Proc<"cpu032I",  [FeatureCpu032I]>;
def : Proc<"cpu032II", [FeatureCpu032II]>;

def Cpu0 : Target {
  let InstructionSet = Cpu0InstrInfo;
}