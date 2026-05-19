add_llvm_component_library(LLVMCpu0Desc
  Cpu0ABIInfo.cpp
  Cpu0MCAsmInfo.cpp
  Cpu0MCTargetDesc.cpp
  Cpu0AsmBackend.cpp
  Cpu0MCCCodeEmitter.cpp
  Cpu0MCExpr.cpp
  Cpu0ELFObjectWriter.cpp

  LINK_COMPONENTS
  MC
  Cpu0AsmPrinter
  Cpu0Info
  Support

  ADD_TO_COMPONENT
  Cpu0
)