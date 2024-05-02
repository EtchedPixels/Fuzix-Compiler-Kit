;
;	,X over Y:D
;
	.export __xdivequl
	.code

	.setcpu 6803

__xdivequl:
	std	@tmp
	ldd	2,x
	pshb
	psha
	ldd	,x
	pshb
	psha
	pshx	; dummy
	ldd	@tmp
	pshb	; save old D at 2,s
	psha
	ldd	@hireg
	pshb	; save hireg at 0,s
	psha
	pshx
	tsx
	inx
	inx	; point to the constructed frame
	jsr	div32x32
	; Divison done now unpack the result
	ldd	6,x
	std	@hireg
	ldd	8,x
	std	@tmp
	pulx
	ldd	@hireg
	std	,x
	ldd	@tmp
	std	2,x
	; Now clean up the mess
	pulx
	pulx
	pulx
	pulx
	pulx
	rts
