;
;	ldx #_case
;	jsr __switchl
;
; _case:
;	.word number of case	; use only lower byte.
;	.word case 1 high word
;	.word case 1 low word
;	.word jmp address 1
;		:
;	.word case N high word
;	.word case N low word
;	.word jmp address N
;	.word default address
;

	.export __switchl

__switchl:
	; X holds the switch table, hireg:D the value
	; Juggle as we are short of regs here - TODO find a nicer approach
	stab @tmp+1
	staa @tmp
	ldab 1,x
	inx
	inx
	tstb
	beq gotit
next:
	stx @tmp2
	ldx 0,x
	cpx @hireg
	bne nomat
	ldx @tmp2
	ldx 2,x
	cpx @tmp
	bne nomat
	ldx @tmp2
	ldx 4,x
	jmp 0,x
nomat:
	ldx @tmp2
	inx
	inx
	inx
	inx
	inx
	inx
	decb			; We know < 256 entries per switch
	bne next
gotit:
	ldx 0,x
	jmp 0,x
