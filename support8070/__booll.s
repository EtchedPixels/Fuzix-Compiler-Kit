	.export __booll
	.export __notl

	.code

__booll:
	or a,e
	ld e,a		; if 0 then now all 0
	or a,:__hireg
	or a,:__hireg+1
	bz doret
true:
	ld ea,=1
doret:
	ret

__notl:
	or a,e
	or a,:__hireg
	or a,:__hireg+1
	bz true
	ld ea,=0
	ret
