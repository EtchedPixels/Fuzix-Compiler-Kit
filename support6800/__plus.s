;
;	Compute TOS + D -> D
;
		.export __plus
		.export __plusu

		.code
__plus:
__plusu:
		tsx
		addb 3,x
		adca 2,x
		ldx 0,x
		ins
		ins
		ins
		ins
		jmp 0,x
