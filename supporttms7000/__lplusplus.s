;
;	++ on a local at offset r13
;
	.export __lplusplus
	.export __lplusplusc
	.export __lplusplusb
	.export __lplusplusbc
	.export __lplusplus1
	.export __lplusplus1c
	.code

__lplusplus1:
	mov %1,r3
__lplusplusb:
	clr r2
__lplusplus:
	clr r12
	add r15,r13
	adc r14,r12
	; Pointer is now in r12/r13 and to last byte, sum to add in r4/r5
	lda *r13
	mov a,r3
	add r5,a
	sta *r13
	decd r13
	lda *r13
	mov a,r2
	adc r4,a
	sta *r13
	movd r2,r4
	rets

__lplusplus1c:
	mov %1,r5
__lplusplusbc:
__lplusplusc:
	clr r12
	add r15,r13
	adc r14,r12
	lda *r13
	mov a,b
	add r5,a
	sta *r13
	mov b,r5
	rets
