;
;		TOS = lval of object L = amount
;
		.export __postincc

		.setcpu 8080
		.code
__postincc:
		xchg
		pop	h
		xthl
		mov	a,m
		mov	d,a
		add	e
		mov	m,a
		mov	e,d
		ret

