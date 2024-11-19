	.export __pop
	.export __popw
	.export __popl

	.export __pop12
	.export __pop10

	.code

__pop:
	lda *r15	; get the byte
	add %1,r15
	adc %0,r14
	sta @0(b)	; into the register indicated
	rets

__pop10:
	mov %10,b
	jmp __popw
__pop12:
	mov %12,b
	jmp __popw
__popl:
	call @__popw
	inc b
__popw:
	lda *r15	; get the byte
	add %1,r15
	adc %0,r14
	sta @0(b)	; into the register indicated
	lda *r15	; get the byte
	add %1,r15
	adc %0,r14
	inc b
	sta @0(b)	; into the register indicated
	rets
	
