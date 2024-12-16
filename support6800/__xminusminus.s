;
;	,X - D
;
	.export __xmminus
	.export __xmminusu
	.code

__xmminus:
__xmminusu:
	staa @tmp
	stab @tmp+1
	ldaa ,x
	ldab 1,x
	subb @tmp+1
	sbca @tmp
	staa ,x
	stab 1,x
	addb @tmp+1
	adca @tmp
	rts
