
	.export __xmuleql
	.export __xmulequl

	.setcpu 6803

__xmuleql:
__xmulequl:
	; Arg in hireg:D ptr in X
	pshx
	pshb
	psha
	ldd @hireg
	pshb
	psha
	ldd ,x
	std @hireg
	ldd 2,x

	; Now we can use the standardc _mull helper

	jsr __mull

	pulx
	; Result is in hireg:D, argument was cleared by called fn
	std 2,x
	ldd @hireg
	std ,x
	ldd 2,x
	rts
