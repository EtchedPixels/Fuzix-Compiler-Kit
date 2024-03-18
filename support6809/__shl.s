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
	tfr d,x
	clrb
	lda 3,x		; low byte to high
shlf:	
	lsla
	leax -1,x
	bne shlf
done:
	ldx 0,s
	leas 4,s
	jmp ,x
nofast:
	tfr d,x
	ldd 2,x
shls:	lslb
	rola
	leax -1,x
	bne shls
	bra done
load_out:
	ldd 2,s
	bra done
bytemove:
	lda 3,s
	clrb
	bra done
