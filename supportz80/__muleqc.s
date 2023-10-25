;
;		(TOS) *= HL
;
		.export __muleqc
		.code

__muleqc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; Now we are doing (HL) * DE
		push	de
		ld	e,(hl)
		ex	(sp),hl
		; We are now doing HL * DE and the address we want is TOS
		call	__mulde
		; Return is in HL
		ex	de,hl
		pop	hl
		ld	(hl),e
		ex	de,hl
		ret
