;
;	 Left shift TOS by D
;

	.export __shl

	.setcpu 6303

__shl:	clra
	andb #15
	beq load_out
	cmpb #8
	beq bytemove
	bcs nofast
	subb #8
	tsx
	ldx 3,x		; low byte to high
	xgdx
	clrb
shlf:	
	lsla
	dex
	bne shlf
done:
	pulx
	ins
	ins
	jmp ,x
nofast:
	tsx
	ldx 2,x
	xgdx
shls:	lslb
	rola
	dex
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
