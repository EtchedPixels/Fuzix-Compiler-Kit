;
;	Until we sort out optimizing this in the compiler proper
;
	.export __minus
	.code

	.setcpu 6803

__minus:
	; Subtract D from TOS
	tsx
	nega
	negb
	sbca #0
	addd 2,x
	pulx
	ins
	ins
	jmp ,x

