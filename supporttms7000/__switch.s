;
;	Switch helper
;
;	On entry index points to the table
;
	.export __switch
	.code

__switch:
	add %1,r13
	adc %0,r12
	lda *r13		; table size
	mov a,b
	jz match
	add %1,r13
	adc %0,r12
switchlp:
	lda *r13
	add %1,r13
	adc %0,r12
	cmp a,r4
	jnz skip3
	lda *r13
	cmp a,r5
	jz match
skip3:
	add %3,r13
	adc %0,r12
	djnz b,switchlp
	jmp def
match:
	add %1,r13
	adc %0,r12
def:
	lda *r13
	mov a,r4
	add %1,r13
	adc %0,r12
	lda *r13
	mov a,r5
	br *r5		
