;
;	These are based ont the CC65 runtime. 6803 is much the same
;	as 6502 here except we've got ldd to load two bytes (and the
;	optimizer will turn it into a 2 byte load of 'one' in the direct page)
;

	.code

	.export booleq
	.export boolne
	.export boolle
	.export boollt
	.export boolge
	.export boolgt
	.export boolule
	.export boolult
	.export booluge
	.export boolugt

	.export __bool
	.export __boolc
	.export __not
	.export __notc

;
;	Turn  val op test into 1 for true 0 for false. Ensure the Z flag
;	is appropriately set
;

__not:
	tsta
	bne	ret0
__notc:
	tstb
booleq:
	bne	ret0
ret1:
	clra
	ldab	#1
	rts

__bool:
	tsta
	bne	ret1
__boolc:
	tstb
boolne:
	bne	ret1
ret0:
	clra
	clrb
	rts

boolle:
	beq	ret1
boollt:
	blt	ret1
	clra
	clrb
	rts

boolge:
	beq	ret1
boolgt:	bgt	ret1
	clra
	clrb
	rts

boolugt:
	beq	ret0
booluge:
	bcc	ret1
	clra
	clrb
	rts

boolule:
	beq	ret1
boolult:			; use C flag
	bcs	ret1
	clra
	clrb
	rts
