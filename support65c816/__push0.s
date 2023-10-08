	.65c816
	.a16
	.i16

	.export __push0
	.export __push1
	.export __push2
	.export __push3
	.export __push4
	.export __pushx

__push0:
	ldx #0
__pushx:
	tya
	sec
	sbc #2
	tay
	stx 0,y
	rts
__push1:
	ldx #1
	bra __pushx
__push2:
	ldx #2
	bra __pushx
__push3:
	ldx #3
	bra __pushx
__push4:
	ldx #4
	bra __pushx
