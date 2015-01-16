; RUN: llc -march=hsail -verify-machineinstrs < %s | FileCheck -check-prefix=HSAIL %s

; HSAIL-LABEL: {{^}}prog function &test_cvt_s32_neari_f32_pat(
; HSAIL: cvt_ftz_neari_s32_f32 {{\$s[0-9]+}}, {{\$s[0-9]+}}
; HSAIL-NEXT: st_arg_align(4)_u32
; HSAIL-NEXT: ret
define i32 @test_cvt_s32_neari_f32_pat(float %x) #1 {
  %round = call float @llvm.rint.f32(float %x) #0
  %cvt = fptosi float %round to i32
  ret i32 %cvt
}

; HSAIL-LABEL: {{^}}prog function &test_cvt_s32_neari_f64_pat(
; HSAIL: cvt_neari_s32_f64 {{\$s[0-9]+}}, {{\$d[0-9]+}}
; HSAIL-NEXT: st_arg_align(4)_u32
; HSAIL-NEXT: ret
define i32 @test_cvt_s32_neari_f64_pat(double %x) #1 {
  %round = call double @llvm.rint.f64(double %x) #0
  %cvt = fptosi double %round to i32
  ret i32 %cvt
}

declare float @llvm.rint.f32(float) #0
declare double @llvm.rint.f64(double) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
