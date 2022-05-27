;
;		TOS = lval of object HL = amount
;
		.export __postincl
		.setcpu 8080
		.code
__postincl:
		xchg
		pop	h
		xthl
		mov	a,m
		sta	__tmp
		add	e
		mov	m,a
		inx	h
		mov	a,m
		sta	__tmp+1
		adc	d
		mov	m,a
		inx	h
		mov	a,m
		sta	__hireg
		adc	d
		mov	m,a
		inx	h
		mov	a,m
		sta	__hireg+1
		adc	d
		mov	m,a
                lhld	__tmp
		ret
