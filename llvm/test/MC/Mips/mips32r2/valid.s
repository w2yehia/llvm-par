# Instructions that are valid
#
# RUN: llvm-mc %s -triple=mips-unknown-linux -show-encoding -mcpu=mips32r2 | FileCheck %s

        .set noat
	abs.d	$f7,$f25 # CHECK: encoding
	abs.s	$f9,$f16
	add	$s7,$s2,$a1
	add.d	$f1,$f7,$f29
	add.s	$f8,$f21,$f24
	addi	$t5,$t1,26322
	addu	$t1,$a0,$a2
	and	$s7,$v0,$t4
	c.ngle.d	$f0,$f16
	c.sf.d	$f30,$f0
	ceil.w.d	$f11,$f25
	ceil.w.s	$f6,$f20
	cfc1	$s1,$21
	clo	$t3,$a1
	clz	$sp,$gp
	ctc1	$a2,$26
	cvt.d.s	$f22,$f28
	cvt.d.w	$f26,$f11
	cvt.s.d	$f26,$f8
	cvt.s.w	$f22,$f15
	cvt.w.d	$f20,$f14
	cvt.w.s	$f20,$f24
	deret
	di	$s8
	div.d	$f29,$f20,$f27
	div.s	$f4,$f5,$f15
	ei	$t6
	eret
	lb	$t8,-14515($t2)
	lbu	$t0,30195($v1)
	ldc1	$f11,16391($s0)
	ldc2	$8,-21181($at)
	ldxc1	$f8,$s7($t7)
	lh	$t3,-8556($s5)
	lhu	$s3,-22851($v0)
	li	$at,-29773
	li	$zero,-29889
	ll	$v0,-7321($s2)
	luxc1	$f19,$s6($s5)
	lw	$t0,5674($a1)
	lwc1	$f16,10225($k0)
	lwc2	$18,-841($a2)
	lwl	$s4,-4231($t7)
	lwr	$zero,-19147($gp)
	lwxc1	$f12,$s1($s8)
	madd	$s6,$t5
	madd	$zero,$t1
	madd.d	$f18,$f19,$f26,$f20
	madd.s	$f1,$f31,$f19,$f25
	maddu	$s3,$gp
	maddu	$t8,$s2
	mfc0	$a2,$14,1
	mfc1	$a3,$f27
	mfhc1	$s8,$f24
	mfhi	$s3
	mfhi	$sp
	mflo	$s1
	mov.d	$f20,$f14
	mov.s	$f2,$f27
	move	$s8,$a0
	move	$t9,$a2
	movf	$gp,$t0,$fcc7
	movf.d	$f6,$f11,$fcc5
	movf.s	$f23,$f5,$fcc6
	movn	$v1,$s1,$s0
	movn.d	$f27,$f21,$k0
	movn.s	$f12,$f0,$s7
	movt	$zero,$s4,$fcc5
	movt.d	$f0,$f2,$fcc0
	movt.s	$f30,$f2,$fcc1
	movz	$a1,$s6,$t1
	movz.d	$f12,$f29,$t1
	movz.s	$f25,$f7,$v1
	msub	$s7,$k1
	msub.d	$f10,$f1,$f31,$f18
	msub.s	$f12,$f19,$f10,$f16
	msubu	$t7,$a1
	mtc1	$s8,$f9
	mthc1	$zero,$f16
	mthi	$s1
	mtlo	$sp
	mtlo	$t9
	mul	$s0,$s4,$at
	mul.d	$f20,$f20,$f16
	mul.s	$f30,$f10,$f2
	mult	$sp,$s4
	mult	$sp,$v0
	multu	$gp,$k0
	multu	$t1,$s2
	neg.d	$f27,$f18
	neg.s	$f1,$f15
	nop
	nor	$a3,$zero,$a3
	or	$t4,$s0,$sp
	round.w.d	$f6,$f4
	round.w.s	$f27,$f28
	sb	$s6,-19857($t6)
	sc	$t7,18904($s3)
	sdc1	$f31,30574($t5)
	sdc2	$20,23157($s2)
	sdxc1	$f11,$t2($t6)
	seb	$t9,$t7
	seh	$v1,$t4
	sh	$t6,-6704($t7)
	sllv	$a3,$zero,$t1
	slt	$s7,$t3,$k1
	slti	$s1,$t2,9489
	sltiu	$t9,$t9,-15531
	sltu	$s4,$s5,$t3
	sqrt.d	$f17,$f22
	sqrt.s	$f0,$f1
	srav	$s1,$s7,$sp
	srlv	$t9,$s4,$a0
	sub	$s6,$s3,$t4
	sub.d	$f18,$f3,$f17
	sub.s	$f23,$f22,$f22
	subu	$sp,$s6,$s6
	suxc1	$f12,$k1($t5)
	sw	$ra,-10160($sp)
	swc1	$f6,-8465($t8)
	swc2	$25,24880($s0)
	swl	$t7,13694($s3)
	swr	$s1,-26590($t6)
	teqi	$s5,-17504
	tgei	$s1,5025
	tgeiu	$sp,-28621
	tlti	$t6,-21059
	tltiu	$ra,-5076
	tnei	$t4,-29647
	trunc.w.d	$f22,$f15
	trunc.w.s	$f28,$f30
	wsbh	$k1,$t1
	xor	$s2,$a0,$s8
