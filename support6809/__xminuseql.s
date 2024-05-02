;
;	,X - Y:D, return new ,X
;
	.export __xminuseql
	.export __xminusequl

	.code
__xminuseql:
__xminusequl:
	pshs y,d	; Value into memory
	ldy ,x		; Get old value into YD
	ldd 2,x
	subd 0,s	; Do the lower subtract
	std 2,x
	exg d,y
	sbcb 3,s	; and upper subtract
	sbca 2,s
	std ,x		; save it
	exg d,y
	leas 4,s	; throw away the temporaries
	rts

