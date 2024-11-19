	.export __cleanup
	.export __cleanupb
	.export __cleanup1
	.export __cleanup2
	.export __cleanup3
	.export __cleanup4
	.export __cleanup5
	.export __cleanup6
	.export __cleanup7
	.export __cleanup8

	.code

__cleanup8:
	mov	%8,r11
	jmp	__cleanupb
__cleanup7:
	mov	%7,r11
	jmp	__cleanupb
__cleanup6:
	mov	%6,r11
	jmp	__cleanupb
__cleanup5:
	mov	%5,r11
	jmp	__cleanupb
__cleanup3:
	mov	%3,r11
	jmp	__cleanupb
__cleanup2:
	mov	%3,r11
	jmp	__cleanupb
__cleanup1:
	mov	%1,r11
	jmp	__cleanupb
__cleanup4:			; 4 is common, 3 is not
	mov	%4,r11
__cleanupb:
	clr	r10
__cleanup:
	add	r11,r15
	adc	r10,r14
	rets
