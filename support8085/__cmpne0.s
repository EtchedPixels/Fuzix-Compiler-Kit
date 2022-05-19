		.export __cmpeq0
		.code

__cmpeq0:
		mov a,h
		ora l
		lxi h,0
		rnz
		inx h	; inx so we leave Z
		ret
