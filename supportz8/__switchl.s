;
;	Switch helper
;
;	On entry index points to the table
;	This one is a bit trickier as r0-r3 are all data, 12/13 are the
;	count and r14/r15 are the working pointer
;
	.export __switchl
	.code

__switchl:
	; r2,r3 is the data
	lde r12,@rr14
	incw rr14
	lde r13,@rr14
	incw rr14
	; 12,13 is the count
	incw rr12
	jr next
switchlp:
	push r12
	lde r12,@rr14
	cp r12,r0
	jr nz,nomatch0
	incw rr14
	lde r12,@rr14
	cp r12,r1
	jr nz,nomatch1
	incw rr14
	lde r12,@rr14
	cp r12,r2
	jr nz,nomatch2
	incw rr14
	lde r12,@rr14
	cp r12,r3
	jr nz,nomatch3
	incw rr14
done:
	pop r12
default:
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	incw rr14
	push r1
	push r0
	ret		; jump into switch handler
nomatch0:
	incw rr14
nomatch1:
	incw rr14
nomatch2:
	incw rr14
nomatch3:
	incw rr14
	incw rr14
	incw rr14
	pop r12
next:
	decw rr12
	jr nz, switchlp
	jr default
