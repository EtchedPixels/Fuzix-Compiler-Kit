;
;	 Right shift TOS by D (unsigned)
;

	.export __shru

__shru:	clra
	andb #15
	beq load_out
	cmpb #8
	beq bytemove
	bcs nofast
	subb #8
	tfr d,x
	clra
	ldb 2,x		; high byte to low
shrf:	
	lsrb
	leax -1,x
	bne shrf
done:
	ldx 0,s
	leas 4,s
	jmp ,x
nofast:
	tfr d,x
	ldd 2,x
shrs:	lsra
	rorb
	leax -1,x
	bne shrs
	bra done
load_out:
	ldd 2,s
	bra done
bytemove:
	ldb 2,s
	clra
	bra done
