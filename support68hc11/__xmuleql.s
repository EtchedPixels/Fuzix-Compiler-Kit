
	.export __xmuleql
	.export __xmulequl

__xmuleql:
__xmulequl:
	; Arg in Y:D ptr in X
	pshx
	pshb
	psha
	pshy
	ldy ,x
	ldd 2,x

	; Now we can use the standardc _mull helper

	jsr __mull

	; Result is in Y:D, argument was cleared by called fn
	pulx
	sty ,x
	std 2,x
	rts
