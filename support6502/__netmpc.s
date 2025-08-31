;
;	Compare A with @tmp
;
	.export __netmpc
	.export __netmpuc

__netmpc:
__netmpuc:
	ldx #0
	cmp @tmp
	beq false	; already 0
true:	lda #1
false:
	rts


