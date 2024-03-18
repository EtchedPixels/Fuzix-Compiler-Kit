;
;	 Right shift TOS by D (signed)
;

	.export __shr

__shr:	clra
	andb #15
	beq load_out
	tfr d,x
	ldd 2,s
shrs:	asra
	rorb
	leax -1,x
	bne shrs
done:	ldx ,s
	leas 4,s
	jmp ,x
load_out:
	ldd 2,s
	bra done
