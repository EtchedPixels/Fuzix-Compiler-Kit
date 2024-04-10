;
;	,X *= D
;
	.export __xmulequc
	.export __xmuleqc

__xmuleqc:
__xmulequc:
	;	Only the low bytes of the input matter
	pshs x		; Save the address
	lda ,x		; Get the ,X value
	pshs a,cc	; Stack it and a spare random byte
	pshs x		; onto the stack
	jsr __mul	; Do the multiply, D gets result
	puls x		; Remove arg from stack
	puls x		; Get the address
	stb ,x		; Save result
	rts
