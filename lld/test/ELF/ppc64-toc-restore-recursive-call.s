# REQUIRES: ppc

# RUN: llvm-mc -filetype=obj -triple=powerpc64le-unknown-linux %s -o %t1.o
# RUN: ld.lld -shared %t1.o -o %t
# RUN: llvm-objdump -d -r %t | FileCheck %s

# For a recursive call that is interposable the linker calls the plt-stub rather
# then calling the function directly. Since the call is through a plt stub and
# might be interposed with a different definition at runtime, a toc-restore is
# required to follow the call.

# The decision to use a plt-stub for the recursive call is not one I feel
# strongly about either way. It was done because it matches what bfd and gold do
# for recursive calls as well as keeps the logic for recursive calls consistent
# with non-recursive calls.

# CHECK-LABEL: __plt_recursive_func:
# CHECK-NEXT: 10000:
# CHECK-LABEL: recursive_func
# CHECK-NEXT:  10014:
# CHECK:       1003c: {{[0-9a-fA-F ]+}} bl .+67108804
# CHECK-NEXT:  ld 2, 24(1)

        .abiversion 2
        .section ".text"
        .p2align 2
        .global recursive_func
        .type recursive_func, @function
recursive_func:
.Lrf_gep:
    addis 2, 12, .TOC.-.Lrf_gep@ha
    addi  2, 2, .TOC.-.Lrf_gep@l
    .localentry recursive_func, .-recursive_func
    cmpldi 3, 2
    blt   0, .Lend

    mflr 0
    std 0, 16(1)
    stdu 1, -32(1)
    addi 5, 3, -1
    mulld 4, 4, 3
    mr 3, 5
    bl recursive_func
    nop
    mr 4, 3
    addi 1, 1, 32
    ld 0, 16(1)
    mtlr 0

.Lend:
    extsw 3, 4
    blr
