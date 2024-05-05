	.export __xplusplusu
	.export __xplusplus
	.code

__xplusplus:
__xplusplusu:
	; Y is free as we are not working in longs
	ldy ,x		; original value
	pshs y		; Save it as a return
	leay d,y	; Y += D in effect
	sty ,x		; Store result
	puls d,pc	; Clean up

