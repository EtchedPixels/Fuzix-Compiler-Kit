;
;		TOS = lval of object HL = amount
;
		.export __postinc
		.code

__postinc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,(hl)
		ld	(__tmp),a
		add	e
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__tmp+1),a
		adc	d
		ld	(hl),a
                ld	hl,(__tmp)
		ret
