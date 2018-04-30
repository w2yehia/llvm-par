; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -instcombine -S | FileCheck %s

; If we have a masked merge, in the form of: (M is constant)
;   ((x ^ y) & M) ^ y
; Unfold it to
;   (x & M) | (y & ~M)

define i4 @scalar0 (i4 %x, i4 %y) {
; CHECK-LABEL: @scalar0(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], 1
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Y]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, 1
  %r  = xor i4 %n1, %y
  ret i4 %r
}

define i4 @scalar1 (i4 %x, i4 %y) {
; CHECK-LABEL: @scalar1(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Y]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, -2
  %r  = xor i4 %n1, %y
  ret i4 %r
}

; ============================================================================ ;
; Various cases with %x and/or %y being a constant
; ============================================================================ ;

define i4 @in_constant_varx_mone(i4 %x, i4 %mask) {
; CHECK-LABEL: @in_constant_varx_mone(
; CHECK-NEXT:    [[R1:%.*]] = or i4 [[X:%.*]], -2
; CHECK-NEXT:    ret i4 [[R1]]
;
  %n0 = xor i4 %x, -1 ; %x
  %n1 = and i4 %n0, 1
  %r = xor i4 %n1, -1
  ret i4 %r
}

define i4 @in_constant_varx_14(i4 %x, i4 %mask) {
; CHECK-LABEL: @in_constant_varx_14(
; CHECK-NEXT:    [[R1:%.*]] = or i4 [[X:%.*]], -2
; CHECK-NEXT:    ret i4 [[R1]]
;
  %n0 = xor i4 %x, 14 ; %x
  %n1 = and i4 %n0, 1
  %r = xor i4 %n1, 14
  ret i4 %r
}

define i4 @in_constant_mone_vary(i4 %y, i4 %mask) {
; CHECK-LABEL: @in_constant_mone_vary(
; CHECK-NEXT:    [[N0:%.*]] = and i4 [[Y:%.*]], 1
; CHECK-NEXT:    [[N1:%.*]] = xor i4 [[N0]], 1
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Y]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %y, -1 ; %x
  %n1 = and i4 %n0, 1
  %r = xor i4 %n1, %y
  ret i4 %r
}

define i4 @in_constant_14_vary(i4 %y, i4 %mask) {
; CHECK-LABEL: @in_constant_14_vary(
; CHECK-NEXT:    [[R:%.*]] = and i4 [[Y:%.*]], -2
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %y, 14 ; %x
  %n1 = and i4 %n0, 1
  %r = xor i4 %n1, %y
  ret i4 %r
}

; ============================================================================ ;
; Commutativity
; ============================================================================ ;

; Used to make sure that the IR complexity sorting does not interfere.
declare i4 @gen4()

define i4 @c_1_0_0 (i4 %x, i4 %y) {
; CHECK-LABEL: @c_1_0_0(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[Y:%.*]], [[X:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Y]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %y, %x ; swapped order
  %n1 = and i4 %n0, -2
  %r  = xor i4 %n1, %y
  ret i4 %r
}

define i4 @c_0_1_0 (i4 %x, i4 %y) {
; CHECK-LABEL: @c_0_1_0(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[X]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, -2
  %r  = xor i4 %n1, %x ; %x instead of %y
  ret i4 %r
}

define i4 @c_0_0_1 () {
; CHECK-LABEL: @c_0_0_1(
; CHECK-NEXT:    [[X:%.*]] = call i4 @gen4()
; CHECK-NEXT:    [[Y:%.*]] = call i4 @gen4()
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X]], [[Y]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[Y]], [[N1]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %x  = call i4 @gen4()
  %y  = call i4 @gen4()
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, -2
  %r  = xor i4 %y, %n1 ; swapped order
  ret i4 %r
}

define i4 @c_1_1_0 (i4 %x, i4 %y) {
; CHECK-LABEL: @c_1_1_0(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[Y:%.*]], [[X:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[X]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %y, %x ; swapped order
  %n1 = and i4 %n0, -2
  %r  = xor i4 %n1, %x ; %x instead of %y
  ret i4 %r
}

define i4 @c_1_0_1 (i4 %x) {
; CHECK-LABEL: @c_1_0_1(
; CHECK-NEXT:    [[Y:%.*]] = call i4 @gen4()
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[Y]], [[X:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[Y]], [[N1]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %y  = call i4 @gen4()
  %n0 = xor i4 %y, %x ; swapped order
  %n1 = and i4 %n0, -2
  %r  = xor i4 %y, %n1 ; swapped order
  ret i4 %r
}

define i4 @c_0_1_1 (i4 %y) {
; CHECK-LABEL: @c_0_1_1(
; CHECK-NEXT:    [[X:%.*]] = call i4 @gen4()
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[X]], [[N1]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %x  = call i4 @gen4()
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, -2
  %r  = xor i4 %x, %n1 ; swapped order, %x instead of %y
  ret i4 %r
}

define i4 @c_1_1_1 () {
; CHECK-LABEL: @c_1_1_1(
; CHECK-NEXT:    [[X:%.*]] = call i4 @gen4()
; CHECK-NEXT:    [[Y:%.*]] = call i4 @gen4()
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[Y]], [[X]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[X]], [[N1]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %x  = call i4 @gen4()
  %y  = call i4 @gen4()
  %n0 = xor i4 %y, %x ; swapped order
  %n1 = and i4 %n0, -2
  %r  = xor i4 %x, %n1 ; swapped order, %x instead of %y
  ret i4 %r
}

define i4 @commutativity_constant_14_vary(i4 %y, i4 %mask) {
; CHECK-LABEL: @commutativity_constant_14_vary(
; CHECK-NEXT:    [[R:%.*]] = and i4 [[Y:%.*]], -2
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %y, 14 ; %x
  %n1 = and i4 %n0, 1
  %r = xor i4 %y, %n1 ; swapped
  ret i4 %r
}

; ============================================================================ ;
; Negative tests. Should not be folded.
; ============================================================================ ;

; One use only.

declare void @use4(i4)

define i4 @n_oneuse_D (i4 %x, i4 %y) {
; CHECK-LABEL: @n_oneuse_D(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Y]]
; CHECK-NEXT:    call void @use4(i4 [[N0]])
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %x, %y ; two uses of %n0, which is going to be replaced
  %n1 = and i4 %n0, -2
  %r  = xor i4 %n1, %y
  call void @use4(i4 %n0)
  ret i4 %r
}

define i4 @n_oneuse_A (i4 %x, i4 %y) {
; CHECK-LABEL: @n_oneuse_A(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Y]]
; CHECK-NEXT:    call void @use4(i4 [[N1]])
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, -2 ; two uses of %n1, which is going to be replaced
  %r  = xor i4 %n1, %y
  call void @use4(i4 %n1)
  ret i4 %r
}

define i4 @n_oneuse_AD (i4 %x, i4 %y) {
; CHECK-LABEL: @n_oneuse_AD(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Y]]
; CHECK-NEXT:    call void @use4(i4 [[N0]])
; CHECK-NEXT:    call void @use4(i4 [[N1]])
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, -2 ; two uses of %n1, which is going to be replaced
  %r  = xor i4 %n1, %y
  call void @use4(i4 %n0)
  call void @use4(i4 %n1)
  ret i4 %r
}

; Mask is not constant

define i4 @n_var_mask (i4 %x, i4 %y, i4 %m) {
; CHECK-LABEL: @n_var_mask(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], [[M:%.*]]
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Y]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, %m
  %r  = xor i4 %n1, %y
  ret i4 %r
}

; Some third variable is used

define i4 @n_third_var (i4 %x, i4 %y, i4 %z) {
; CHECK-LABEL: @n_third_var(
; CHECK-NEXT:    [[N0:%.*]] = xor i4 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[N1:%.*]] = and i4 [[N0]], -2
; CHECK-NEXT:    [[R:%.*]] = xor i4 [[N1]], [[Z:%.*]]
; CHECK-NEXT:    ret i4 [[R]]
;
  %n0 = xor i4 %x, %y
  %n1 = and i4 %n0, -2
  %r  = xor i4 %n1, %z ; not %x or %y
  ret i4 %r
}
