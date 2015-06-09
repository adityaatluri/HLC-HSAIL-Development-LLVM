; RUN: llc -march=hsail -verify-machineinstrs < %s | FileCheck -check-prefix=HSAIL %s

declare i32 @llvm.hsail.sad.u32.u32(i32, i32, i32) #0

; HSAIL-LABEL: {{^}}prog function &test_sad_u32_i32(
; HSAIL: sad_u32_u32 {{\$s[0-9]+}}, {{\$s[0-9]+}}, {{\$s[0-9]+}}, {{\$s[0-9]+}};
define i32 @test_sad_u32_i32(i32 %x, i32 %y, i32 %z) #0 {
  %val = call i32 @llvm.hsail.sad.u32.u32(i32 %x, i32 %y, i32 %z) #0
  ret i32 %val
}

; HSAIL-LABEL: {{^}}prog function &test_sad_u32_i32_rii(
; HSAIL: sad_u32_u32 {{\$s[0-9]+}}, {{\$s[0-9]+}}, 16777216, 2;
define i32 @test_sad_u32_i32_rii(i32 %x) #0 {
  %val = call i32 @llvm.hsail.sad.u32.u32(i32 %x, i32 16777216, i32 2) #0
  ret i32 %val
}

; HSAIL-LABEL: {{^}}prog function &test_sad_u32_i32_rir(
; HSAIL: sad_u32_u32 {{\$s[0-9]+}}, {{\$s[0-9]+}}, 1, {{\$s[0-9]+}};
define i32 @test_sad_u32_i32_rir(i32 %x, i32 %y) #0 {
  %val = call i32 @llvm.hsail.sad.u32.u32(i32 %x, i32 1, i32 %y) #0
  ret i32 %val
}

; HSAIL-LABEL: {{^}}prog function &test_sad_u32_i32_rri(
; HSAIL: sad_u32_u32 {{\$s[0-9]+}}, {{\$s[0-9]+}}, {{\$s[0-9]+}}, 7;
define i32 @test_sad_u32_i32_rri(i32 %x, i32 %y) #0 {
  %val = call i32 @llvm.hsail.sad.u32.u32(i32 %x, i32 %y, i32 7) #0
  ret i32 %val
}

; HSAIL-LABEL: {{^}}prog function &test_sad_u32_i32_iri(
; HSAIL: sad_u32_u32 {{\$s[0-9]+}}, 342421, {{\$s[0-9]+}}, 9;
define i32 @test_sad_u32_i32_iri(i32 %x) #0 {
  %val = call i32 @llvm.hsail.sad.u32.u32(i32 342421, i32 %x, i32 9) #0
  ret i32 %val
}

; HSAIL-LABEL: {{^}}prog function &test_sad_u32_i32_iir(
; HSAIL: sad_u32_u32 {{\$s[0-9]+}}, 256, 65536, {{\$s[0-9]+}};
define i32 @test_sad_u32_i32_iir(i32 %x) #0 {
  %val = call i32 @llvm.hsail.sad.u32.u32(i32 256, i32 65536, i32 %x) #0
  ret i32 %val
}

; HSAIL-LABEL: {{^}}prog function &test_sad_u32_i32_iii(
; HSAIL: sad_u32_u32 {{\$s[0-9]+}}, 3, 1234, 11;
define i32 @test_sad_u32_i32_iii() #0 {
  %val = call i32 @llvm.hsail.sad.u32.u32(i32 3, i32 1234, i32 11) #0
  ret i32 %val
}

attributes #0 = { nounwind readnone }
