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
	ldaa 2,x
	ldab 3,x
	subb @tmp+1
	sbca @tmp
	staa 2,x
	stab 3,x
	addb @tmp+1
	adca @tmp
	rts
