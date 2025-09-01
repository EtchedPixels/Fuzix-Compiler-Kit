;
;	The DG Nova 3 has no direct byte operations.
;
;	Our rules of operation are as follows
;
;	reads can leave the upper byte random
;	writes need to clear the upper byte and cannot assume it
;	is clear
;
;	This rule helps us minimise the amount of work we have do masking
;	and clearing
;
	.export f__derefc
	.export f__derefuc
	.export f__assignc

	.export f__pluseqc
	.export f__eqcget
	.export f__equcget

	.code

;
;	1 holds the byte pointer and gets the data, 0 and 2 can be trashed
;	3 is restored as the fp
;
f__derefc:			; OBSOLETE: REMOVE ? OR KEEP FOR -Os
f__derefuc:
	sta	3,__tmp,0	; save return address
	mov	1,2
	lda	0,N255,0	; set ac0 with the mask
	movr	2,2,snc		; word address skip if right byte sets up C
	movs	0,0		; swap mask if needed
	lda	1,0,2		; get word into 1
	and	0,1,snc		; we now have the right bits masked
				; snc will skip depending which half we need
	movs	1,1		; swap if needed
	mffp	3		; restore frame pointer
	jmp	@__tmp,0

f__assignc:
	popa	2
	sta	3,__tmp,0
	lda	3,N255,0
	and	3,1		; mask input
	movr	2,2,snc		; as before figure the addr, set up carry
	movs	1,1,skp		; flip word around
	movs	3,3		; swap mask if we need to
	lda	0,0,2		; get existing word
	and	3,0		; mask off
	add	0,1		; add to new byte
	sta	1,0,2		; put back
	mffp	3
	jmp	@__tmp,0

; byte at AC2 += AC1
f__pluseqc:
	popa	2		; byte pointer
	sta	3,__tmp,0
	movr	2,2,szc		; get actual word, decide side
	jmp	loplus,1	; carry zero high byte
	movs	1,1
	lda	0,0,2
	add	0,1
	sta	1,0,2		; write the value back
	movs	1,1		; put result into low byte
	mffp	3
	jmp	@__tmp,0
loplus:
	lda	0,0,2
	add	0,1		; 1 is now value we need to merge
	lda	0,N255,0
	and	0,1		; mask to low byte to clear overflows
	movs	0,0		; flip mask other way (FF00)
	lda	3,0,2		; get original
	and	0,3		; mask it
	add	0,1		; merge
	sta	1,0,2
	mffp	3
	jmp	@__tmp,0

;
;	General byte eqop support
;	jsr @__eqcget
;	jsr @__someop
;	jsr @__assignc
;
;	Passed a pointer on the stack which should be kept, must preserve
;	AC1, stack deref result and return it in AC2
;
f__eqcget:
	popa	2		; byte address
	psha	2		; keep it on stack for later
	psha	3		; save return for assignc to use
	sta	1,__tmp2,0	; save working value
	mov	2,1
	jsr	f__derefc,1	; fetch byte we need
	jsr	@__castc_
	popa	3		; get return back
	psha	1		; save fetched value (expression left)
	mov	1,2		; core code wants a copy to save in AC2
	lda	1,__tmp2,0	; recover expression right
	sta	3,__tmp,0	; and return
	mffp	3
	jmp	@__tmp,0

f__equcget:
	popa	2		; byte address
	psha	2		; keep it on stack for later
	psha	3		; save return for assignc to use
	sta	1,__tmp2,0	; save working value
	mov	2,1
	jsr	f__derefc,1	; fetch byte we need
	popa	3		; get return back
	psha	1		; save fetched value (expression left)
	mov	1,2		; core code wants a copy to save in AC2
	lda	1,__tmp2,0	; recover expression right
	sta	3,__tmp,0	; and return
	mffp	3
	jmp	@__tmp,0
