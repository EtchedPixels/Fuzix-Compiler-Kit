;
;	,X - Y:D, return old ,X
;
	.export __xmminusl
	.export __xmminusul

	.code
__xmminusl:
__xmminusul:
	pshs y,d	; Value into memory
	ldy ,x		; Get old value into YD
	ldd 2,x
	pshs y,d	; Save old value
	subd 6,s	; Do the lower subtract
	std 2,x
	exg d,y
	sbcb 5,s	; and upper subtract
	sbca 4,s
	std ,x		; save it
	puls y,d	; recover the original value
	leas 4,s	; throw away the temporaries
	rts

