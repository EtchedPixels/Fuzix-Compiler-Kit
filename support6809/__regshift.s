	.export __reglsl
	.export __regshr
	.export __regshru

	.code

__reglsl:
	clra
	andb	#15
	exg	d,u
lsln:
	lslb
	rola
	leau	-1,u
	bne	lsln
	tfr	d,u
	rts

__regshru:
	clra
	andb	#15
	exg	d,u
lsrn:
	lsra
	rorb
	leau	-1,u
	bne	lsrn
	tfr	d,u
	rts

__regshr:
	clra
	andb	#15
	exg	d,u
asrn:
	asra
	rorb
	leau	-1,u
	bne	asrn
	tfr	d,u
	rts
