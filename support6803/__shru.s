;
;	 Right shift TOS by D (unsigned)
;

	.export __shru

	.setcpu 6803
__shru:	clra
	andb #15
	beq load_out
	cmpb #8
	beq bytemove
	bcs nofast
	subb #8
	stab @tmp
	tsx
	ldab 2,x
	clra
shrf:	
	lsrb
	dec @tmp
	bne shrf
done:
	pulx
	ins
	ins
	jmp ,x
nofast:
	stab @tmp
	tsx
	ldd 2,x
shrs:	lsra
	rorb
	dec @tmp
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
