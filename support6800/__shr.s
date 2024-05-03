;
;	 Right shift TOS by D (signed)
;
	.export __shr

__shr:	clra
	andb #15
	beq load_out
	stab @tmp
	tsx
	ldaa 2,x
	ldab 3,x
shrs:	asra
	rorb
	dec @tmp
	bne shrs
done:	jmp __pop2
load_out:
	tsx
	ldaa 2,x
	ldab 3,x
	jmp __pop2