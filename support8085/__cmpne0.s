		.export __cmpeq0
		.export __cmpeq0b
		.code

__cmpeq0b:
		mvi h,0
__cmpeq0:
		mov a,h
		ora l
		lxi h,0
		rnz
		inx h	; inx so we leave Z
		ret
