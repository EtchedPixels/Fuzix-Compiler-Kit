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
	clr a
remdivul:
	movd r15,r13	; r12 points to data
	call @__div32x32
	; Pull 4 off stack
	jmp __cleanup4

__reml:
	mov %1,a	; signed remainder
	jmp remdivul

__divul:
	mov %2,a	; divide
	jmp remdivul

__divl:
	mov %3,a	; signed divide
	jmp remdivul

__remequl:
	call @__pop12	; address
	clr a
eqstore:
	call @__div32x32
	mov r5,a
	sta *r13
	decd r13
	mov r4,a
	sta *r13
	decd r13
	mov r3,a
	sta *r13
	decd r13
	mov r2,a
	sta *r13
	rets

__remeql:
	call @__pop12
	mov %1,a
	jmp eqstore

__divequl:
	call @__pop12
	mov %2,a
	jmp eqstore

__diveql:
	call @__pop12
	mov %3,a
	jmp eqstore


; Divide @r12 by r2-r5 working regs r10-r13
; r4-r7 stacked restored and preserved
; on entry a bit 0 is set if signed, bit 1 set if divide not remainder
; r12/13 on entry point to the object we will use as the dividend. On exit
; they point to the last byte of the object. The helpers for /= etc rely
; upon this.

__div32x32:
	push r9		; Save register variables
	push r8
	push r7
	push r6
	lda *r13
	mov a,r6
	add %1,r13
	adc %0,r12
	lda *r13
	mov a,r7
	add %1,r13
	adc %0,r12
	lda *r13
	mov a,r8
	add %1,r13
	adc %0,r12
	lda *r13
	mov a,r9
	push r13
	push r12

	clr b		; sign tracking
	clrc
	rrc a		; set C based on signed/unsigned
	jnc is_unsigned	; at this point A is zero for remainder
				; one for divide.

	or r2,r2
	jp uns1
	or a,a		; Check if divide or mod
	jz is_mod
	inc b		; Remember sign change for divides

is_mod:
	inv r2
	inv r3
	inv r4
	inv r5
	add %1,r5
	adc %0,r4
	adc %0,r3
	adc %0,r2
uns1:
	or r6,r6
	jp is_unsigned
	inc b
	inv r6		; invert the dividend
	inv r7
	inv r8
	inv r9
	add %1,r9
	adc %0,r8
	adc %0,r7
	adc %0,r6
;
;	We do the maths using
;	R2-R5: divisor
;	R6-R9: dividend
;	R10-R13: working register (ends up result)
;	B: counter
;	A: info bits
;
;	Nothing is preserved
;
is_unsigned:
	clr r10		; clear the workiing register
	clr r11
	clr r12
	clr r13
	push b		; save the sign info (could be in A ?)
	mov %32,b
divl:
	; dividend left
	add r9,r9
	adc r8,r8
	adc r7,r7
	adc r6,r6
	; Rotate into working value
	rlc r13
	rlc r12
	rlc r11
	rlc r10
	; Compare with divisor
	cmp r2,r10
	jnz divl2
	cmp r3,r10
	jnz divl2
	cmp r4,r12
	jnz divl2
	cmp r5,r13
divl2:	jnc skipadd
	; Subtract from working value
	sub r5,r13
	sbb r4,r12
	sbb r3,r11
	sbb r2,r10
	; Set low bit in rotating r6-r9 (bit is currently 0)
	inc r9
skipadd:
	djnz b,divl
	; Result in r6-r9, remainder in r10-r13
	; info bits in A
	or a,a
	jz is_rem
	movd r5,r1
	movd r7,r3
	jmp mod_result
is_rem:
	movd r9,r1
	movd r11,r3	; We want the remainder
mod_result:
	; Result is now in r0-r3
	pop b		; Get the sign info back

	pop r12		; Restore pointer
	pop r13
	pop r6		; Recover register variables
	pop r7
	pop r8
	pop r9

	; Check if we need to negate the result
	or b,b
	jz no_invert
	inv r2
	inv r3
	inv r4
	inv r5
	add %1,r5
	adc %0,r4
	adc %0,r3
	adc %0,r2
no_invert:
	rets

