;
;	Switch helper
;
;	On entry index points to the table
;
	.export __switchc
	.code

__switchc:
	; r3 is the data
	ldei r12,@rr14
	ldei r13,@rr14
	; 12,13 is the count
	ld r0,r12
	or r0,r13
	jr z,done
switchlp:
	ldei r0,@rr14
	cp r0,r3
	jr nz, nomatch
done:
	ldei r0,@rr14
	ldei r1,@rr14
	push r1
	push r0
	ret		; jump into switch handler
nomatch:
	incw rr14
	incw rr14
	decw rr12
	jr nz, switchlp
	jr done
