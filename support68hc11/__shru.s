;
;	 Right shift TOS by D (unsigned)
;

	.export __shru

__shru:	tsy
	clra
	andb #15
	beq load_out
	cmpb #8
	beq bytemove
	bcs nofast
	subb #8
	xgdx
	clra
	ldab 2,y		; high byte to low
shrf:	
	lsrb
	dex
	bne shrf
done:
	puly
	pulx
	jmp ,y
nofast:
	xgdx
	ldd 2,y
shrs:	lsra
	rorb
	dex
	bne shrs
	bra done
load_out:
	ldd 2,y
	bra done
bytemove:
	ldab 2,y
	clra
	bra done
