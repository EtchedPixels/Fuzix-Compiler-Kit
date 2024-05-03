;
;	Long complement. Possibly inlinable
;
	.export __cpll

	.code
__cpll:
	coma
	comb
	staa @tmp
	stab @tmp+1
	ldaa @hireg
	ldab @hireg+1
	coma
	comb
	staa @hireg
	stab @hireg+1
	ldaa @tmp
	ldab @tmp+1
	rts
