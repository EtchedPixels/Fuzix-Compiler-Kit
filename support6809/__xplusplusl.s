;
;	Preserve original value as result
;	Messy need to think how to do it better
;
	.export __xplusplusl
	.export __xplusplusul

	.code
__xplusplusl:
__xplusplusul:
	std ,--s		; save original D
	ldd ,x
	std ,--s		; save original high
	ldd 2,x
	std ,--s		; save original low
	ldd 4,s			; get original D back
	; add Y,D to ,X
	addd 2,x		; add low words
	std 2,x			; save
	bcc no_overflow
	leay 1,y		; carry one if needed
no_overflow:
	exg d,y			; now do high words
	addd 0,x		; add original high
	std 0,x			; store back
	puls d,y		; recover 
	leas 2,s
	rts
