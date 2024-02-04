	.65c816
	.a16
	.i16

	.export __mulx
	.export __mulxu

;
;	We ought to do this the other way around and optimize for
;	only clear bits left. TODO
;
;	There are also a bunch of other better algorithms! FIXME
;

__mulxu:
__mulx:	; calculate A * X
	stx @tmp	; amount to add
	sta @tmp2	; value we shift through
	lda #0		; sum
	ldx #16		; iterations
loop:
	asl a		; double working sum
	asl @tmp2	; next bit
	bcc noadd	; if zero no work this loop
	clc
	adc @tmp
noadd:	dex		; Loop done
	bne loop
	; Result is in A
	rts
