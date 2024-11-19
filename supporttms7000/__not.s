	.export __notl
	.export __not
	.export __notc
	

__notl:
	or r3,r0
	or r3,r1
__not:
	or r3,r2
	clr r2
	jz true
	xor r3,r3
	rets
true:	; r3 is currently 0
	inc r3	; sets flags right too
	rets
__notc:
	or r3,r3
	clr r2
	jz true
	xor r3,r3
	rets
	