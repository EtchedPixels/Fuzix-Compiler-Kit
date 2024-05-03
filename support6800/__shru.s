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
	stab @tmp
	ldab 2,x
	clra
shrf:	
	lsrb
	dec @tmp
	bne shrf
	jmp __pop2
nofast:
	stab @tmp
	tsx
	ldaa 2,x
	ldab 3,x
shrs:	lsra
	rorb
	dec @tmp
	bne shrs
	jmp __pop2
load_out:
	tsx
	ldaa 2,x
	ldab 3,x
	jmp __pop2
bytemove:
	tsx
	ldab 2,x
	clra
	jmp __pop2
