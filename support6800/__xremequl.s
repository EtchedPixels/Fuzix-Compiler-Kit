;
;	,X over hireg:D
;
	.export __xremequl
	.code

	.setcpu 6803
__xremequl:
	std	@tmp
	ldd	2,x
	pshb
	psha
	ldd	,x
	pshb
	psha
	pshx	; dummy
	ldd	@tmp
	pshb	; Save D at 2,S in frame
	psha
	ldd	@hireg
	pshb
	psha	; save hireg at 0,S in the frame
	pshx
	tsx
	inx
	inx	; X points at the frame
	jsr	div32x32
	ldd	@tmp2
	std	@hireg
	pulx
	ldd	@tmp2
	std	,x
	ldd	@tmp3
	std	2,x
	pulx
	pulx
	pulx
	pulx
	pulx	; clean up and done
	rts
