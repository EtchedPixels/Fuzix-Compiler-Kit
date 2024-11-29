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
	addb 3,x
	adca 2,x
	jmp __pop2
