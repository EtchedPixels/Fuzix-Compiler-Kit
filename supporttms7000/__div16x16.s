;
;	16 x 16 divide core
;
	.export __div16x16
	.export __divu
	.export __remu

	.code


; Divide r2-r3 by r4-r5
__div16x16:
	clr r10					; work
	clr r11
	mov %16,a				; tmp
divl:
	add r3,r3	; shift dividend left	; X
	adc r2,r2
	rlc r11		; rotate into work	; work D
	rlc r10
	; Is work bigger than divisor
	cmp r4,r10				; D v tmp1
	jnz divl2
	cmp r5,r11
divl2:	jnc skipadd				; 
	sub r5,r11				; - divisor
	sbb r4,r10
	inc r3		; set low bit of r3 (we shifted it so it is 0 atm)
skipadd:
	djnz a,divl
	;	At this point r10,r11 is the remainder
	;	r2,r3 is the quotient
	rets

__divu:
	call @__pop2
	call @__div16x16 ; do the division
	movd r3,r5
	rets

__remu:
	call @__pop2
	call @__div16x16
	movd r11,r5
	rets
