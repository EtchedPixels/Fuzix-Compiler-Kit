;
;	Until we sort out optimizing this in the compiler proper
;
	.export __minus
	.code

	.setcpu 6803

__minus:
	; Subtract D from TOS
	tsx
	coma
	comb
	addd @one
	addd 2,x
	pulx
	ins
	ins
	jmp ,x

