;
;	32bit multipliers
;
	.export __mulequl
	.export __mulul
	.export __muleql
	.export __mull

__muleql:
__mulequl:
	; stack holds ret then ptr, registers hold value
	pop	r12
	pop	r13
	pop	r14
	pop	r15	; r14/15 is ptr
	push	r13
	push	r12	; ret addr back

	call	domull

	lde	@rr14,r3
	decw	rr14
	lde	@rr14,r2
	decw	rr14
	lde	@rr14,r1
	decw	rr14
	lde	@rr14,r0

	ret

__mull:
__mulul:
	ld	r14,254		; SP
	ld	r15,255
	add	r15,#2		; get pointer to data on stack into r14/15
	adc	r14,#0

	call	domull		; result is now in r0-r3, 12-15 trashed

	pop	r12		; return address
	pop	r13
	add	255,#4		; discard from stack
	adc	254,#0

	push	r13		; and done
	push	r12
	ret


domull:
	; Make room to think
	push	r7
	push	r6
	push	r5
	push	r4

	; 0-3 is amount, 4-7 will be sum
	; 14/15 are ptr as we do each byte
	; 12 is the working byte, 13 is the count in byte

	clr	r4
	clr	r5
	clr	r6
	clr	r7

	; Do this in four chunks loading a byte at a time to keep the
	; register pressure down

	lde	r12,@rr14	; get first byte of value
	call	mulbyte
	incw	rr14
	lde	r12,@rr14
	call	mulbyte
	incw	rr14
	lde	r12,@rr14
	call	mulbyte
	incw	rr14
	lde	r12,@rr14
	call	mulbyte

	; 4 8x32 slices done. Result is now in 4-7
	ld	r0,r4
	ld	r1,r5
	ld	r2,r6
	ld	r3,r7

	pop	r4
	pop	r5
	pop	r6
	pop	r7
	ret
;
;	Do a 32x8 slice of multiplication
;
mulbyte:
	ld	r13,#8
mulbl:
	add	r7,r7		; shift total
	adc	r6,r6
	adc	r5,r5
	adc	r4,r4

	add	r12,r12		; rotate working byte
	jr	nc, noadd

	add	r7,r3		; add in the multiplier
	adc	r6,r2
	adc	r5,r1
	adc	r4,r0
noadd:
	djnz	r13, mulbl
	ret
