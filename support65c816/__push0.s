	.65c816
	.a16
	.i16

	.export __push0
	.export __push1
	.export __push2
	.export __push3
	.export __push4
	.export __pusha
	.export __pushal
	.export __pushl0
	.export __pushl0a
	.export __pushy0
	.export __pushy2
	.export __pushy4
	.export __pushy6
	.export __pushy8
	.export __pushy10

__push0:
	lda #0
	; Pusha must preserve A and X (the compiler knows this)
__pusha:
	dey
	dey
	sta 0,y
	rts
__push1:
	lda #1
	bra __pusha
__push2:
	lda #2
	bra __pusha
__push3:
	lda #3
	bra __pusha
__push4:
	lda #4
	bra __pusha

__pushal:
	; Preserves A and X
	dey
	dey
	dey
	dey
	sta 0,y
	pha
	lda @hireg
	sta 2,y
	pla
	rts

__pushl0:
	lda #0
__pushl0a:
	; preserves A and X
	dey
	dey
	dey
	dey
	stz 2,y
	sta 0,y
	rts

__pushy0:
	lda 0,y
	bra __pusha
__pushy2:
	lda 2,y
	bra __pusha
__pushy4:
	lda 4,y
	bra __pusha
__pushy6:
	lda 6,y
	bra __pusha
__pushy8:
	lda 8,y
	bra __pusha
__pushy10:
	lda 10,y
	bra __pusha
