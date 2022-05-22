		.export __shrde
		.export __shrdeu
		.export __shru
		.setcpu 8085
		.code

__shru:
		xchg		; shift value into d
		pop	h	; return address into h
		xthl		; return up stack, h is now the value to shift

; We have a 16bit right signed shift only
__shrdeu:
		mov	a,h
		ora	a
		jm	shrneg
__shrde:
		mov	a,e
		ora	a
		rz		; no work to do
shrpl:
		arhl
shnext:
		dcr	a
shftr:		jnz	shrpl
		ret


; Negative number - do first shift, clear the top bit then fall into the
; main loop
shrneg:
		mov	a,e
		ora	a
		rz		; no work to do
		arhl
		mov	a,h
		ani	0x7F
		mov	h,a
		jmp	shnext
