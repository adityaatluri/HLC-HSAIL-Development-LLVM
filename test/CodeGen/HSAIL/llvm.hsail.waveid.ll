; RUN: llc -march=hsail -verify-machineinstrs < %s | FileCheck -check-prefix=HSAIL -check-prefix=FUNC %s

declare i32 @llvm.hsail.waveid() #0
declare i32 @llvm.HSAIL.get.dynwave.id() #0

; FUNC-LABEL: {{^}}prog function &test_waveid(
; HSAIL: waveid_u32 {{\$s[0-9]+}};
define void @test_waveid(i32 addrspace(1)* %out) #1 {
  %tmp0 = call i32 @llvm.hsail.waveid() #0
  store i32 %tmp0, i32 addrspace(1)* %out
  ret void
}

; FUNC-LABEL: {{^}}prog function &test_legacy_get_dynwave_id(
; HSAIL: waveid_u32 {{\$s[0-9]+}};
define void @test_legacy_get_dynwave_id(i32 addrspace(1)* %out) #1 {
  %tmp0 = call i32 @llvm.HSAIL.get.dynwave.id() #0
  store i32 %tmp0, i32 addrspace(1)* %out
  ret void
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
