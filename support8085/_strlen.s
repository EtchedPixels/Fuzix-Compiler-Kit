;
;	strlen
;
		.export _strlen
		.setcpu 8080
		.code

_strlen:
	pop	d
	pop	h
	push	h
	push	d
	lxi	d,0
loop:
	mov	a,m
	inx	h
	ora	a
	rz
	inx	d
	jmp	loop
