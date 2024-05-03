;
;	 Left shift TOS by D
;

	.export __shl

__shl:	clra
	andb #15
	beq load_out
	cmpb #8
	beq bytemove
	bcs nofast
	subb #8
	stab @tmp
	tsx
	ldaa 3,x		; low byte to high
	clrb
shlf:	
	lsla
	dec @tmp
	bne shlf
done:
	jmp __pop2
nofast:
	stab @tmp
	tsx
	ldaa 2,x
	ldab 3,x
shls:	lslb
	rola
	dec @tmp
	bne shls
	jmp __pop2
load_out:
	tsx
	ldaa 2,x
	ldab 3,x
	jmp __pop2
bytemove:
	tsx
	ldaa 3,x
	clrb
	jmp __pop2