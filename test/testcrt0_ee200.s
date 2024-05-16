;
;	EE200 wrapper for testing
;
	.code

	lda 0xE000
	xas
	jsr _main
	stbb (0xFFFF)
