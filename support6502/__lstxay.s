	.export __lstaxy
	.code

; Compiler knows this increments Y

__lstaxy:
	pha
	sta (@sp),y
	txa
	iny
	sta (@sp),y
	pla
	rts
