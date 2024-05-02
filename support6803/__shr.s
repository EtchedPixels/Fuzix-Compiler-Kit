;
;	 Right shift TOS by D (signed)
;
	.export __shr

	.setcpu 6803

__shr:	clra
	andb #15
	beq load_out
	stab @tmp
	tsx
	ldd 2,x
shrs:	asra
	rorb
	dec @tmp
	bne shrs
done:	pulx
	ins
	ins
	jmp ,x
load_out:
	tsx
	ldd 2,x
	bra done
