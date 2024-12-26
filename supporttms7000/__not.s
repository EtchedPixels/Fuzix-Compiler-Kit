	.export __notl
	.export __not
	.export __notc
	

__notl:
	or r5,r2
	or r5,r3
__not:
	or r5,r4
	clr r4
	jz true
	clr r5
	rets
true:	; r5 is currently 0
	inc r5	; sets flags right too
	rets
__notc:
	clr r4
	or r5,r5
	jz true
	clr r5
	rets
	