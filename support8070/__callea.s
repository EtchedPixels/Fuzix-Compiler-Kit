;
;	Stub for doing a jsr ea
;
	.export __callea

__callea:
	sub ea,=1		; because we need to allow for the
				; pre-inc of the PC
	ld p0,ea		; PC = EA
