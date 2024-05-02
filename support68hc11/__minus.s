;
;	Until we sort out optimizing this in the compiler proper
;
	.export __minus
	.code

__minus:
	; Subtract D from TOS
	tsy
	coma
	comb
	addd @one
	addd 2,y
	puly
	pulx
	jmp ,y
