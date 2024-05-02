;
;	,X over Y:D
;
	.export __xremequl
	.code
__xremequl:
	pshx	; Make space for the frame expected
	pshx
	pshx
	pshb	; Save D at 2,S in frame
	psha
	pshy	; save Y at 0,S in the frame
	tsy
	ldd	,x
	std	6,y
	ldd	2,x
	std	8,y
	pshx
	tsx
	inx
	inx	; X points at the frame
	jsr	div32x32
	ldy	@tmp2
	ldd	@tmp3
	pulx
	sty	,x
	std	2,x
	pulx
	pulx
	pulx
	pulx
	pulx	; clean up and done
	rts
