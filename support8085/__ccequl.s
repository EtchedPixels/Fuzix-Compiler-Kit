			.export __ccequl
			.setcpu 8080
			.code

__ccequl:
	call	__cmpulws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jz	__true
	jmp	__false

