;
;	 Right shift TOS by D (signed)
;

	.export __shr

__shr:	tsy
	clra
	andb #15
	beq load_out
	xgdx
	ldd 2,y
shrs:	asra
	rorb
	dex
	bne shrs
done:	puly
	pulx
	jmp ,y
load_out:
	ldd 2,y
	bra done
