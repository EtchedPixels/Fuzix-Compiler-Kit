;
;	Preserve original value as result
;	Messy need to think how to do it better
;
	.export __xplusplusl
	.export __xplusplusul

	.code
__xplusplusl:
__xplusplusul:
	std ,--s
	ldd ,x
	std ,--s
	ldd 2,x
	std ,--s
	; add Y,D to ,X
	addd 2,x
	std 2,x
	bcc no_overflow
	leay 1,y
no_overflow:
	exg d,y
	addd 0,x
	std 0,x
	puls d,y
	leas 2,s
	rts
