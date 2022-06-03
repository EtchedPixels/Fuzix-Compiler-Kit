		.export __shrdeu
		.export __shru
		.setcpu 8080
		.code

__shru:
		xchg		; shift value into d
		pop	h	; return address into h
		xthl		; return up stack, h is now the value to shift

; We have a 16bit right signed shift only
__shrdeu:
		mov	a,e
		ani	15
		rz		; no work to do
		mov	e,a
shrlp:
		mov	a,h
		ora	a
		rar
		mov	h,a
		mov	a,l
		rar
		mov	l,a
shnext:
		dcr	e
shftr:		jnz	shrlp
		ret

