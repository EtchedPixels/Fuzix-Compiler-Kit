;
;	Stub crt for running tests
;
	call	_main
	mov	a,l
	out	0xFF

	.export	_printint

_printint:
	pop	h
	pop	d
	push	d
	mov	a,e
	out	0xFC
	mov	a,d
	out	0xFD
	pchl

	.export _printchar
_printchar:
	pop	h
	pop	d
	push	d
	mov	a,e
	out	0xFE
	pchl
