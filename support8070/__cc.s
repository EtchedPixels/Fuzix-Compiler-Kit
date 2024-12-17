;
;	16bit condition code helpers
;

	.export __ceqeqw
	.export __cbangeqw
	.export __ceqeqwu
	.export __cbangeqwu

	.export __cgteqwu
	.export __clteqwu
	.export __cgtwu
	.export __cltwu

	.export __cgteqw
	.export __clteqw
	.export __cgtw
	.export __cltw

	.code

__ceqeqw:
__ceqeqwu:
	or	a,e
	bz	true
false:
	ld	ea,=0
	ret

__cbangeqw:
__cbangeqwu:
	or	a,e
	bz	false
true:
	ld	ea,=1
	ret

__cgteqwu:
	ld	a,s
	bp	true
	ld	ea,=0
	ret

__cltwu:
	ld	a,s
	bp	false
	ld	ea,=1
	ret

__cgtwu:
	or	a,e
	bz	false
	ld	a,s
	bp	true
	ld	ea,=0
	ret

__clteqwu:
	or	a,e
	bz	true
	ld	a,s
	bp	false
	ld	ea,=1
	ret

; These test OV

__cgteqw:
	and	s,=0x40
	ld	a,s
	bz	true
	ld	ea,=0
	ret

__cltw:
	and	s,=0x40
	ld	a,s
	bz	false
	ld	ea,=1
	ret

__cgtw:
	or	a,e
	bz	false
	and	s,=0x40
	ld	a,s
	bz	true
	ld	ea,=0
	ret

__clteqw:
	or	a,e
	bz	true
	and	s,=0x40
	ld	a,s
	bz	false
	ld	ea,=1
	ret
