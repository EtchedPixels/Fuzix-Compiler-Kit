			.export __ccneul
			.setcpu 8080
			.code

__ccneul:
	call	__cmpulws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jz	__false
	jmp	__true


