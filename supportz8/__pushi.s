;
;	Push integers and long. Compact form for -Os as the usual long
; 	form is painfully longwinded on Z8
;
;	Leaves the value in the accumulator as expected
;	Trashes only r14/r15

	.export __pushil
	.export __pushil0
	.export __pushi

	.code

__pushil:
	pop r14
	pop r15			; get return ptr
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	incw rr14
	lde r2,@rr14
	incw rr14
	lde r3,@rr14
	incw rr14
	push r3
	push r2
	push r1
	push r0
	jp @rr14

__pushil0:
	pop r14
	pop r15			; get return ptr
	clr r0
	clr r1
	lde r2,@rr14
	incw rr14
	lde r3,@rr14
	incw rr14
	push r3
	push r2
	push r1
	push r0
	jp @rr14

__pushi:
	pop r14
	pop r15
	lde r2,@rr14
	incw rr14
	lde r3,@rr14
	incw rr14
	push r3
	push r2
	jp @rr14

