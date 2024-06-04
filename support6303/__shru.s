;
;	 Right shift TOS by D (unsigned)
;

	.export __shru

	.setcpu 6303
__shru:	clra
	andb #15
	beq load_out
	cmpb #8
	beq bytemove
	bcs nofast
	subb #8
	tsx
	ldx 1,x	; high byte into low of X will become B
	xgdx
	clra
shrf:	
	lsrb
	dex
	bne shrf
done:
	pulx
	ins
	ins
	jmp ,x
nofast:
	tsx
	ldx 2,x
	xgdx
shrs:	lsra
	rorb
	dex
	bne shrs
	bra done
load_out:
	tsx
	ldd 2,x
	bra done
bytemove:
	tsx
	ldab 2,x
	clra
	bra done
