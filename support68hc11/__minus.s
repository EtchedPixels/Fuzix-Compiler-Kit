;
;	Until we sort out optimizing this in the compiler proper
;
	.export __minus
	.code

__minus:
	; Subtract D from TOS
	tsx
	nega
	negb
	sbca #0
	addd 2,x
	pulx
	puly
	jmp ,x
