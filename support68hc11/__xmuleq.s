;
;	,X *= D
;
	.export __xmuleq
	.export __xmulequ

__xmuleq:
__xmulequ:
	pshx		; Save the address
	ldx ,x		; Get the ,X value
	pshx		; onto the stack
	jsr __mul	; Do the multiply, D gets result, _mul removes the arg
	pulx		; Get the address
	std ,x		; Save result
	rts
