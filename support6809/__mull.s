;
;	Multiply 3,x by sreg:d
;
;
;	Stack on entry
;
;	3-7,x	Argument
;
;
		.export __mull
		.export __mulul

		.code

__mull:
__mulul:
	leas	-4,s		; working space ,x->3,x
				; moves argument to 6-9,x
	std	@tmp
;
;	Do 32bit x low 8bits
;	This gives us a 32bit result plus 8bits discarded
;
	lda	9,s
	ldb	@tmp+1
	mul			; calculate low 8 and overflow
	std	2,s		; fill in low 16bits
	lda	8,s
	ldb	@tmp+1
	mul
	addb	2,s		; next 16 bits plus carry
	adca	#0
	std	1,s		; and save
	lda	7,s
	ldb	@tmp+1
	mul
	addb	1,s		; next 16bits plus carry
	adca	#0
	std	,s		; and save
	lda	6,s
	ldb	@tmp+1
	mul			; top 8bits (and overflow is lost)
	addb	,s
	stb	,s
;
;	Now repeat this with the next 8bits but we only need to do 24bits
;	as the top 8 will overflow
;
	lda	9,s
	ldb	@tmp
	mul
	addb	2,s		; add in the existing
	adca	1,s
	std	1,s
	bcc	norip16
	inc	,s		; carry into the top byte
norip16:
	lda	8,s
	ldb	@tmp	; again
	mul
	addb	1,s
	adca	,s
	std	,s
	lda	7,s
	ldb	@tmp
	mul
	addb	,s
	stb	,s	; rest overflows, all of top byte overflows
;
;	Now repeat for the next 8bits but we only need to do 16bit as the
;	top 16 will overflow. Spot a 16bit zero and short cut as this
;	is common (eg for uint * ulong cases)
;
	tfr	y,d	; again (b = sreg + 1)
	beq	is_done
	lda	9,s
	mul
	addb	1,s
	adca	,s
	std	,s
	tfr	y,d
	lda	8,s
	mul
	addb	,s
	stb	,s	; rest overflows, all of top byte overflows
;
;	And finally the top 8bits so almost everything overflows
;
	tfr	y,d
	beq	is_done
	ldb	9,s
	mul
	addb ,s
	stb ,s		; rest overflows, all of top byte overflows
;
;	Into our working register
;
is_done:
	ldy ,s++
	ldd ,s++
	ldx ,s++
	leas 4,s
	jmp ,x
