;
;	,X over Y:D
;
	.export __xdivequl
	.code
__xdivequl:
	pshx
	pshx
	pshx
	pshb	; save D at 2,s
	psha
	pshy	; save Y at 0,s
	tsy
	ldd	,x
	std	6,y
	ldd	2,x
	std	8,y
	pshx
	tsx
	inx
	inx	; point to the constructed frame
	jsr	div32x32
	; Divison done now unpack the result
	ldy	6,x
	ldd	8,x
	pulx
	sty	,x
	std	2,x
	; Now clean up the mess
	pulx
	pulx
	pulx
	pulx
	pulx
	rts
