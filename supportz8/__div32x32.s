	.export __remul
	.export __divul
	.export __reml
	.export __divl
	.export __remequl
	.export __remeql
	.export __divequl
	.export __diveql

	.code

__remul:
	clr r13
remdivul:
	ld r14,254
	ld r15,255
	incw r14
	incw r14	; points to data
	call __div32x32
	pop r12
	pop r13
	add 254,#4
	adc 255,#0
	push r13
	push r12
	ret	

__reml:
	ld r13,#1	; signed remainder
	jr remdivul

__divul:
	ld r13,#2	; divide
	jr remdivul

__divl:
	ld r13,#3	; signed divide
	jr remdivul

__remequl:
	pop r12
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	clr r12
eqstore:
	call __div32x32
	lde @rr14,r3
	decw rr14
	lde @rr14,r2
	decw rr14
	lde @rr14,r1
	decw rr14
	lde @rr14,r0
	ret

__remeql:
	pop r12
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	ld r12,#1
	jr eqstore

__divequl:
	pop r12
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	ld r12,#2
	jr eqstore

__diveql:
	pop r12
	pop r13
	pop r14
	pop r15
	push r13
	push r12
	ld r12,#3
	call __div32x32
	jr eqstore


; Divide @rr14 by r0-r3 working regs r8-r11, also uses r12/13 r14/15 preserved
; r4-r7 stacked restored and preserved
; on entry r12 bit 0 is set if signed, bit 1 set if divide not remainder

__div32x32:
	push r11
	push r10
	push r9
	push r8
	push r7
	push r6
	push r5
	push r4
	lde r4,@rr14
	incw r14
	lde r5,@rr14
	incw r14
	lde r6,@rr14
	incw r14
	lde r7,@rr14
	clr r13		; sign tracking
	rrc r12
	jr nc, is_unsigned

	tcm r0,#0x80
	jr z,uns1
	inc r13
	com r0
	com r1
	com r2
	com r3
	add r3,#1
	adc r2,#0
	adc r1,#0
	adc r0,#0
uns1:
	tcm r4,#0x80
	jr z, is_unsigned
	or r12,r12
	jr z, is_mod
	inc r13
is_mod:
	com r4
	com r5
	com r6
	com r7
	add r7,#1
	adc r6,#0
	adc r5,#0
	adc r4,#0
is_unsigned:
	clr r8
	clr r9
	clr r10
	clr r11
	push r13		; save the sign info
	ld r13,#32
divl:
	; dividend left
	add r3,r3
	adc r2,r2
	adc r1,r1
	adc r0,r0
	; Rotate into working value
	rlc r11
	rlc r10
	rlc r9
	rlc r8
	; Compare with divisor
	cp r8,r4
	jr nz, divl2
	cp r9,r5
	jr nz, divl2
	cp r10,r6
	jr nz, divl2
	cp r11,r7
divl2:	jr c, skipadd
	; Add to working value
	add r11, r7
	adc r10, r6
	adc r9, r5
	adc r8, r4
	; Set low bit in rotating r3-r0 (bit is currently 0)
	inc r0
skipadd:
	djnz r13,divl
	; Result in r8-r11, remainder in r0-r3
	pop r4
	pop r5
	pop r6
	pop r7
	or r12,r12
	jr z, is_rem
	ld r0,r8
	ld r1,r9
	ld r2,r10
	ld r3,r11
is_rem:
	; Result is now in r0-r3
	pop r13
	; Check if we need to negate the result
	or r13,r13
	jr z, no_invert
	com r0
	com r1
	com r2
	com r3
	add r3,#1
	adc r2,#0
	adc r1,#0
	adc r0,#0
no_invert:
	pop r8
	pop r9
	pop r10
	pop r11
	ret

