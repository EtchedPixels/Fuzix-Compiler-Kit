;
;	Helper for size/low opt settings to do loads
;
;	These functions all only affect the accumulator and r14/15
;	They *must* leave r14/15 where the compiler expects as it has
;	internal knowledge of this
;
	.export __gargr1
	.export __gargr2
	.export __gargr4
	.export __gargrr1
	.export __gargrr2
	.export __gargrr4
	.export __load2
	.export __load4

__gargr1:
	clr	r14
__gargrr1:
	add	r15,255
	adc	r14,254
	lde	r3,@rr14
	ret

__gargr2:
	clr	r14
__gargrr2:
	add	r15,255
	adc	r14,254
__load2:
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	ret

__gargr4:
	clr	r14
__gargrr4:
	add	r15,255
	adc	r14,254
__load4:
	lde	r0,@rr14
	incw	rr14
	lde	r1,@rr14
	incw	rr14
	lde	r2,@rr14
	incw	rr14
	lde	r3,@rr14
	ret

	.export __revstore4
	.export __revstore2

__revstore4:
	lde	@rr14,r3
	decw	rr14
	lde	@rr14,r2
	decw	rr14
	lde	@rr14,r1
	decw	rr14
	lde	@rr14,r0
	ret

__revstore2:
	lde	@rr14,r3
	decw	rr14
	lde	@rr14,r2
	ret

	.export __store4
	.export __store2

__store4:
	lde	@rr14,r0
	incw	rr14
	lde	@rr14,r1
	incw	rr14
	lde	@rr14,r2
	incw	rr14
	lde	@rr14,r3
	ret

__store2:
	lde	@rr14,r2
	incw	rr14
	lde	@rr14,r3
	ret
