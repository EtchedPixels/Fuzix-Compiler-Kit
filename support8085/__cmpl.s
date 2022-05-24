;
;	Compare two objects in memory unsigned and set C NC and Z
;	accordingly. Eats the two pointers in H and D
;
			.export __cmpul
			.export __cmpulws
			.export __cmpl
			.export __cmplws
			.setcpu 8080
			.code

__cmpulws:		; workspace and stack
	shld	__tmp	; word before __hireg
	lxi	h,4	; stacked value
	dad	sp
	lxi	d,__tmp
__cmpul:
	ldax	d
	cmp	m
	rnz
	inx	d
	inx	h
	ldax	d
	cmp	m
	rnz
	inx	d
	inx	h
	ldax	d
	cmp	m
	rnz
	inx	d
	inx	h
	ldax	d
	cmp	m
	rnz
	inx	d
	inx	h
	ldax	d
	cmp	m
	ret

__cmplws:
	shld	__tmp
	lxi	h,4
	dad	sp
	lxi	d,__tmp
__cmpl:
	; Same idea but we need to deal with signs first
	ldax	d
	xra	m
	; Same sign - unsigned compare
	jp	__cmpul
	xra	m
	jp	setnc	; m +ve / d -ve
	ora	a	; clear carry
	; We know A top bit is set so NZ is as we want it
	ret
setnc:	xra	a
	inr	a	; NZ
	scf		; NZ and C
	ret
	
