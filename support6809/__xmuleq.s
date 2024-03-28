;
;	,X *= D
;
	.export __xmuleq

__xmuleq:
	pshs x		; Save the address
	ldx ,x		; Get the ,X value
	pshs x		; onto the stack
	jsr __mul	; Do the multiply, D gets result
	puls x		; Remove arg from stack
	puls x		; Get the address
	std ,x		; Save result
	rts
