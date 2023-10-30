		.export __cmpeq
		.export __cmpeq0d
		.export __cmpeqla
		.code

;
;	Tighter version with the other value in DE
;
__cmpeq0d:
		ld	d,0
__cmpeq:
		or	a
		sbc	hl,de
		jp	nz,__false
		;	was 0 so now 1 and NZ
		inc	l
		ret
; Used by optimizer peephole to squash 8 v 8 compare into cp a with l
__cmpeqla:
		sub	l
		jp	nz, __false	; true if they are the same
		ld	hl,0
		inc	l		; was 0 now 1 and NZ
		ret
