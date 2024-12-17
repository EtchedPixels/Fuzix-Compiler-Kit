;
;	Complement XA
;
	.export __cpl

__cpl:
	pha
	txa
	eor #0xFF
	tax
	pla
	eor #0xFF
	rts
