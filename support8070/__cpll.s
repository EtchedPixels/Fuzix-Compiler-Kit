;
;	32bit complement and negation
;
	.export __cpll
	.export __negatel

__cpll:
	xor a,=255
	xch a,e
	xor a,=255
	xch a,e
	ld t,ea
	ld ea,:__hireg
	xor a,=255
	xch a,e
	xor a,=255
	xch a,e
	st ea,:__hireg
	ld ea,t
	ret

__negatel:
	jsr __cpll
	add ea,=1
	ld t,ea
	rrl a		; get carry bit
	bp done		; no carry no mess
	ld ea,:__hireg
	add ea,=1
	st ea,:__hireg
done:
	ld ea,t
	ret
