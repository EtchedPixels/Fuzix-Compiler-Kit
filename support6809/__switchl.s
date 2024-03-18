	.export __switchl

__switchl:
	; X holds the switch table, Y:D the value
	stx ,--s		; Top of stack is now pointer to counter
	inc [1,s]
	bra moveon
next:
	cmpy ,x++
	bne bump
	cmpd ,x++
	beq gotit
	leax 2,x
moveon:
	dec [1,s]		; We know < 256 entries per switch
	bne next
gotit:
	ldx ,x
	leas 2,s
	jmp ,x
bump:	leax 4,x
	bra moveon
