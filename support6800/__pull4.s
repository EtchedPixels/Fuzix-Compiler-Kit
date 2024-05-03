
	.export __pull4
	.code

__pull4:
	tsx
	ldx ,x
	ins
	ins
	pula
	pulb
	staa @hireg
	stab @hireg+1
	pula
	pulb
	jmp ,x
