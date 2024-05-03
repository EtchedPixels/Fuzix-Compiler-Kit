;
;	,X *= D
;
	.export __xmuleq
	.export __xmulequ

__xmuleq:
__xmulequ:
	stx @tmp4
	staa @tmp
	stab @tmp+1
	ldaa ,x		; Get the ,X value
	ldab 1,x
	pshb		; onto the stack
	psha
	ldaa @tmp
	ldab @tmp+1
	jsr __mul	; Do the multiply, D gets result, _mul removes the arg
	ldx @tmp4
	staa ,x		; Save result
	stab 1,x
	rts
