;
;	 Left shift TOS by D
;

	.export __shl

	.setcpu 6803

__shl:	clra
	andb #15
	beq load_out
	cmpb #8
	beq bytemove
	bcs nofast
	subb #8
	stab @tmp
	tsx
	ldd 3,x		; low byte to high
	clrb
shlf:	
	lsla
	dec @tmp
	bne shlf
done:
	pulx
	inx
	inx
	jmp ,x
nofast:
	stab @tmp
	tsx
	ldd 2,x
shls:	lslb
	rola
	dec @tmp
	bne shls
	bra done
load_out:
	tsx
	ldd 2,x
	bra done
bytemove:
	tsx
	ldaa 3,x
	clrb
	bra done
