;
;	Call the function in XA
;
	.export __callfunc

__callfunc:
;	This is messier than we'd like because rts increments the target
;	to add to the fun you can only push A on the 6502 classic. Thus
;	it's easier to take the hit of keeping a JMP byte somewhere and
;	patching. That however breaks horribly on a 65C816 if you've got
;	different code and data pages - GAK
	sec
	sbc	#1
	bcs	l1
	dex
	; XA is now the address we want to push. Play pass the parcel so
	; we can get them on the stack in the right order
	; TODO: if we do a C02 lib version we can just phx pha rts at the
	; end
	tay
	txa
	pha
	tya
	pha
	rts
