;
;	Compute TOS + hireg:D -> hireg:D
;
		.export __plusl
		.export __plusul

		.code
__plusl:
__plusul:
		tsx
		addb 5,x
		adca 4,x
		staa @tmp
		ldaa @hireg+1
		adca 3,x
		staa @hireg+1
		ldaa @hireg
		adca 2,x
		staa @hireg
		ldaa @tmp
		ldx 0,x
		ins
		ins
		ins
		ins
		ins
		ins
		jmp 0,x
