;
;	Nova 3 test
;
	.code
start:
	lda	0,spinit,1
	mtsp	0
	jsr	@1,1
	.word	_main
	nio	0		; our exit trap device
spinit:
	.word	0x7000		; grows upwards
