;
;	 Left shift TOS by D
;

	.export __shl

__shl:	tsy
	clra
	andb #15
	beq load_out
	cmpb #8
	beq bytemove
	bcs nofast
	subb #8
	xgdx
	clrb
	ldaa 3,y		; low byte to high
shlf:	
	lsla
	dex
	bne shlf
done:
	puly
	pulx
	pulx
	jmp ,y
nofast:
	xgdx
	ldd 2,y
shls:	lslb
	rola
	dex
	bne shls
	bra done
load_out:
	ldd 2,y
	bra done
bytemove:
	ldaa 3,y
	clrb
	bra done
