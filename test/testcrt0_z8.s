
	.code
	.ds 12		; vectors
start:
	; Stack Ptr FF00 (FEFF first byte of stack)
	clr 0xFF
	ld 0xFE,#0xFF
	; P01M Stack external, high address bits enable
	ld 0xF8,#0x92
	srp #0x10
	call __reginit
	call _main
	; Write result to FFFF external data
	ld r14,#0xFF
	ld r15,r14
	lde @rr14,r3
