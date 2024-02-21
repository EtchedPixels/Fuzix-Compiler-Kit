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
	clr r12
remdivul:
	ld r14,254
	ld r15,255
	incw r14
	incw r14	; points to data
	call __div32x32
	pop r12
	pop r13
	add 255,#4
	adc 254,#0
	push r13
	push r12
	ret	

__reml:
	ld r12,#1	; signed remainder
	jr remdivul

__divul:
	ld r12,#2	; divide
	jr remdivul

__divl:
	ld r12,#3	; signed divide
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
	jr eqstore


; Divide @rr14 by r0-r3 working regs r8-r11, also uses r12/13 r14/15 preserved
; r4-r7 stacked restored and preserved
; on entry r12 bit 0 is set if signed, bit 1 set if divide not remainder
; r14/15 on entry point to the object we will use as the dividend. On exit
; they point to the last byte of the object. The helpers for /= etc rely
; upon this.

__div32x32:
	push r11
	push r10
	push r9
	push r8
	push r7
	push r6
	push r5
	push r4
	lde r4,@rr14	; dividend
	incw r14
	lde r5,@rr14
	incw r14
	lde r6,@rr14
	incw r14
	lde r7,@rr14
	clr r13		; sign tracking
	rcf
	rrc r12		; set C based on signed/unsigned
	jr nc, is_unsigned	; at this point R12 is zero for remainder
				; one for divide.

	tcm r0,#0x80	; invert the divisor if needed
	jr nz,uns1
	or r12,r12
	jr z, is_mod
	inc r13		; and remember that
is_mod:
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
	jr nz, is_unsigned
	inc r13
	com r4		; invert the dividend
	com r5
	com r6
	com r7
	add r7,#1
	adc r6,#0
	adc r5,#0
	adc r4,#0
;
;	We do the maths using
;	R0-R3: divisor
;	R4-R7: dividend
;	R8-R11: working register (ends up result)
;	R13: counter
;	Preserves 12,14,15
;
is_unsigned:
	clr r8		; clear the workiing register
	clr r9
	clr r10
	clr r11
	push r13		; save the sign info
	ld r13,#32
divl:
	; dividend left
	add r7,r7
	adc r6,r6
	adc r5,r5
	adc r4,r4
	; Rotate into working value
	rlc r11
	rlc r10
	rlc r9
	rlc r8
	; Compare with divisor
	cp r8,r0
	jr nz, divl2
	cp r9,r1
	jr nz, divl2
	cp r10,r2
	jr nz, divl2
	cp r11,r3
divl2:	jr c, skipadd
	; Subtract from working value
	sub r11, r3
	sbc r10, r2
	sbc r9, r1
	sbc r8, r0
	; Set low bit in rotating r4-r7 (bit is currently 0)
	inc r7
skipadd:
	djnz r13,divl
	; Result in r4-r7, remainder in r8-11
	or r12,r12
	jr z, is_rem
	ld r0,r4	; We want quotient
	ld r1,r5
	ld r2,r6
	ld r3,r7
	jr mod_result
is_rem:
	ld r0,r8	; We want the remainder
	ld r1,r9
	ld r2,r10
	ld r3,r11
mod_result:
	; Result is now in r0-r3
	pop r13		; Get the sign info back
	pop r4		; Recover register variables
	pop r5		; we stashed earlier
	pop r6
	pop r7
	pop r8
	pop r9
	pop r10
	pop r11
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
	ret
