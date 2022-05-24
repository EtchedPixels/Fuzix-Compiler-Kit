;
;		True and False helpers. Set H and flags
;
		.export __true
		.export	__false
		.export __rtrue
		.export	__rfalse

__true:
		lxi	h,0
		inr	h
		ret
__false:
		xra	a
		mov	h,a
		mov	l,a
		ret

__rtrue:
		lxi	h,0
		inr	h
		jmp	__ret

__rfalse:	xra	a
		mov	h,a
		mov	l,a
		jmp	__ret
