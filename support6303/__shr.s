;
;	 Right shift TOS by D (signed)
;
	.export __shr

	.setcpu 6303

__shr:	clra
	andb #15
	beq load_out
	tsx
	ldx 2,x
	xgdx
shrs:	asra
	rorb
	dex
	bne shrs
done:	pulx
	ins
	ins
	jmp ,x
load_out:
	tsx
	ldd 2,x
	bra done
