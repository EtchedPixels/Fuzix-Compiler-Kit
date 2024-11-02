;
;	Push 
;
	.zp

	.export push
	.export pushl
	.export pushconst

push:	.word	__push
pushl:	.word	__pushl
pushconst:
	.word	__pushconst

	.code
;
;	Push A onto @SP. We have no subtract one op and no way to modify X
;	so this gets a bit messy
;
__push:	.word 0
	sta *@sp
	cra
	cma
	add @sp
	sta @sp
	jmp *__push

__pushl: .word 0
	sta *@sp
	cra
	cma
	add @sp
	sta @sp
	iba
	sta *@sp
	cr
	cma
	add @sp
	sta @sp
	jmp *__pushl

__pushconst: .word 0
	lda *__pushconst
	irs __pushconst	
	; won't skip
	sta *@sp
	cra
	cma
	add @sp
	sta @sp
	jmp *__pushconst

	.commondata

	.word __push
	.word __pushl
	.word __pushconst
