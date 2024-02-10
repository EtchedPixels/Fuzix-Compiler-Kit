;
;	Switch helper
;
;	On entry index points to the table
;
	.export __switch
	.code

__switch:
	; r2,r3 is the data
	lde r12,@rr14
	incw rr14
	lde r13,@rr14
	incw rr14
	; 12,13 is the count
switchlp:
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	incw rr14
	cp r0,r2
	jr nz, nomatch
	cp r1,r3
	jr nz, nomatch
done:
	lde r0,@rr14
	incw rr14
	lde r1,@rr14
	incw rr14
	push r1
	push r0
	ret		; jump into switch handler
nomatch:
	incw rr14
	incw rr14
	decw rr12
	jr nz, switchlp
	jr done
