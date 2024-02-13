#
;
;	First draft of the support code to run the compiler
;	as a threadcode generator for 1802/5/6 (there's a lot of
;	possible optimizing for 1805/6)
;
;	We make some assumptions for simplicity
;	- Our working stack is not R2. We leave that for asm helpers and
;	  interrupts as we don't always keep SP safe to directly use
;	- Our interrupt entry switches threadcode stacks so the state of the
;	  user SP isn't that important and can be briefly slightly invalid
;
	.abs
	.org 0x100

#include "1802tab.s"

;		0	; DMA
;		1	; Interrupt handler
;		2	; System stack for interrupt
#define AC	3	; Accumulator low
#define HI	4	; Accumulator high
#define SP	5	; Thread code stack (must bump down 4 bytes before
			; reusing in interrupts)
#define BPC	6	; Threadcode program counter
#define PC	7	; Program counter for the thread op
#define RUN	8	; Program counter for the main co-routine
#define TMP	9	; Working scratch (also PC for native ?)
#define FP	10	; Frame pointer
#define VP	11	; Vector table pointer (must be page aligned)
			; 12-15 free for register variables
#define	R_0	12
#define R_1	13
#define R_2	14
#define R_3	15
;
;	1802 engine
;
op_shl:
	glo AC
	ani 15
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
	inc SP
	lda SP
	phi AC
	br  lslnext
lslloop:
	glo AC
	shl
	plo AC
	ghi AC
	shlc
	phi AC
lslnext:
	dec TMP
	glo TMP
	bnz lslloop
	sep RUN

op_shr:
	; We should do smart things for 8bit shift
	glo AC
	ani 15
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
	inc SP
	ldn SP
	phi AC
	shl
	bdf lsrnext
	br asrnext
asrloop:
	ghi AC
	shr
	adi 0x80
	phi AC
	glo AC
	shrc
	plo AC
asrnext:
	dec TMP
	glo TMP
	bnz asrloop
	sep RUN
op_shru:
	; We should do smart things for 8bit shift
	glo AC
	ani 15
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
	inc SP
	ldn SP
	phi AC
	br lsrnext
lsrloop:
	ghi AC
	shr
	phi AC
	glo AC
	shrc
	plo AC
lsrnext:
	dec TMP
	glo TMP
	bnz lsrloop
	sep RUN
op_shift1:
	ldi >byteop2
	phi VP
	sep RUN
op_pushc:
	glo AC
	str SP
	dec SP
	sep RUN
op_push:
	sex SP
	ghi AC
	stxd
	glo AC
	stxd
	sex BPC
	sep RUN
op_popl:
	inc SP
	inc SP
op_pop:
	inc SP
op_popc:
	inc SP
	sep RUN
op_plus:
	sex SP
	inc SP
	glo AC
	add
	plo AC
	inc SP
	ghi AC
	adc
	phi AC
	sex BPC
	sep RUN
op_minus:	; TOS - AC
	sex SP
	inc SP
	glo AC
	sd
	plo AC
	inc SP
	ghi AC
	sdb
	phi AC
	sex BPC
	sep RUN
op_negate:
	glo AC
	xri 0xFF
	plo AC
	ghi AC
	xri 0xFF
	phi AC
	inc AC
	sep RUN
op_band:
	sex SP
	inc SP
	glo AC
	and
	plo AC
	inc SP
	ghi AC
	and
	phi AC
	sex SP
	sep RUN
op_or:
	sex SP
	inc SP
	glo AC
	or
	plo AC
	inc SP
	ghi AC
	or
	phi AC
	sex SP
	sep RUN
op_xor:
	sex SP
	inc SP
	glo AC
	xor
	plo AC
	inc SP
	ghi AC
	xor
	phi AC
	sex SP
	sep RUN
op_cpll:
	glo HI
	xri 0xFF
	plo HI
	ghi HI
	xri 0xFF
	phi HI
op_cpl:	glo AC
	xri 0xFF
	plo AC
	ghi AC
	xri 0xFF
	plo AC
	sep RUN
op_assignc:
	inc SP
	ldn SP
	str AC
	plo AC		; stored value becomes working
	sep RUN
op_assign:
	inc SP
	lda SP
	str AC
	plo TMP
	ldn SP
	str AC
	phi AC
	glo TMP
	plo AC
	sep RUN
op_derefc:
	lda AC
	plo AC
	sep RUN
op_deref:
	lda AC
	plo TMP
	lda AC
	phi AC
	glo TMP
	plo AC
	sep RUN
op_xxeq:		; start of += etc. Duplicate the pointer top of stack
	inc SP
	lda SP
	plo TMP
	phi TMP
	dec SP
	dec SP
	glo TMP
	stxd
	ghi TMP
	stxd
	sep RUN
op_xxeqpostc:		; po the address and write back the result
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	glo AC
	str TMP
	sep RUN
op_xxeqpost:		; po the address and write back the result
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	glo AC
	str TMP
	inc TMP
	ghi AC
	str TMP
	sep RUN
op_xxeqpostl:		; po the address and write back the result
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	glo AC
	str TMP
	inc TMP
	ghi AC
	str TMP
	inc TMP
	glo HI
	str TMP
	inc TMP
	ghi HI
	str TMP
	sep RUN
op_constc:
	ldxa
	plo AC
	sep RUN
op_const:
	ldxa
	plo AC
	ldxa
	phi AC
	sep RUN
op_notl:
	glo HI
	bnz set0
	ghi HI
	bnz set0
op_not:
	ghi AC
	bnz set0
op_notc:
	glo AC
	bnz set0
	; is 0 should be 1
	inc AC
	sep RUN
set0:
	ldi 0
	phi AC
	ldi 1
	plo AC
	sep RUN
;
;	Now the fun stuff - 16bit divide
;
op_rem:
	sex SP
	ghi AC
	shl
	bz op_remu	; Remainder sign matches divisor
			; so low means same as unsigned
	ldi 0x80	; Remember to invert the sign
	br negates
op_div:
	sex SP
	ghi AC
	inc SP
	inc SP		; point at high byte
	xor
	shl		; DF tells us resulting sign
	dec SP
	dec SP		; fix the stack
	ldi 0x01
	lsnf
	ldi 0x81
	phi HI		; Remember what we are doing
negates:
	; Now turn both halves +ve
	ghi AC
	shl
	bnf no_neg_1
	glo AC
	xri 0xFF
	plo AC
	ghi AC
	xri 0xFF
	phi AC
	inc AC
no_neg_1:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	shl
	bnf no_neg_2
	glo TMP
	xri 0xFF
	plo TMP
	ghi TMP
	xri 0xFF
	phi TMP
	inc TMP
	; Now put it back on the stack and SP back as it should be
	ghi TMP
	stxd
	glo TMP
	stxd
no_neg_2:
	br __div16x16

op_remu:
	ldi 0
	lskp
op_divu:
	ldi 1
	phi HI
;
;	This point is entered with HI.hi indicating if the remainder
;	or quotient should be returned and whether to invert the result
;
;	This allows it to be entered for all the 16bit divide/remainder
; forms
;
__div16x16:
	; TOS / AC -> AC
	; need to shuffle stuff around so divisor is in memory

	inc SP
	ldn SP
	plo TMP		; tmp is the dividend
	glo AC		; Swap with our extra register save
	str SP
	inc SP
	ldn SP
	phi TMP
	ghi AC
	str SP		; leave sp pointing at high byte of divisor
	ldi 16
	plo HI

	ldi 0		; working register
	plo AC
	phi AC


loop:
	glo TMP
	shl
	plo TMP
	ghi TMP
	shlc
	phi TMP
	; Now do we set the low bit on the working reg ?
	inc TMP		; guess yes
	ghi AC
	sm
	bdf next_loop
	bnz do_adjust
	; Might be work to do
	glo AC
	dec SP		; divisor low
	sm
	inc SP		; back to divisor high
	bdf next_loop
	bz  next_loop
do_adjust:
	dec SP		; divisior low
	glo AC
	add
	plo AC
	inc SP		; divisor high
	ghi AC
	adc
	dec TMP		; we guessed wrong
next_loop:
	dec HI
	glo HI
	bnz loop

	; State at this point
	; HI = 0
	; AC holds the remainder
	; (TMP) holds the dividend
	; Our callers stuck info in HI.HI about what to return
	ghi HI
	ani 1
	bz divide
	; Remainder. AC is good
chksign:
	ghi HI
	ani 0x80
	bz done
	glo AC
	; Negate requested
	xri 0xFF
	plo AC
	ghi AC
	xri 0xFF
	phi AC
	inc AC
done:
	; Now clean up  (SP already points to upper byte we work with)
	inc SP
	sex BPC
	sep RUN
divide:
	glo TMP
	plo AC
	ghi TMP
	phi AC
	br chksign
op_mul:
	; AC * TOS
	glo SP
	plo HI
	ghi SP
	phi HI		; HI now points the value to work with
	sex HI

	; AC * (HI)
	ldi 0
	plo TMP2	; sum
	phi TMP2

nextbits:
	glo TMP2	; sum << = 1
	shl
	plo TMP2
	ghi TMP2
	shlc
	phi TMP2
	ghi AC		; top bit set
	shl
	bz noadd
	glo TMP2
	add
	inc HI
	plo TMP2
	ghi TMP2
	adc
	phi TMP2
	dec HI
noadd:
	; now shift bits along
	glo AC
	bnz morebits
	ghi AC
	bnz morebits
	glo TMP2
	plo AC
	ghi TMP2
	phi AC
	sex BPC
	sep RUN
morebits:
	glo AC
	shl
	plo AC
	ghi AC
	shlc
	phi AC
	br nextbits

op_booll:
	ghi HI
	bnz set1
	glo HI
	bnz set1
op_bool:
	ghi AC
	bnz set1
op_boolc:
	glo AC
	lsz
set1:
	ldi 1
	plo AC
	ldi 0
	phi AC
	sep RUN
op_extc:
	glo AC
	shl
	bdf op_extuc
	ldi 255
	phi AC
	plo HI
	phi HI
	sep RUN
op_ext:
	ghi AC
	shl
	bdf op_extu
	ldi 255
	plo HI
	phi HI
	sep RUN
op_extuc:
	ldi 0
	phi AC
op_extu:
	ldi 0
	plo HI
	phi HI
	sep RUN
op_f2l:
op_l2f:
op_f2ul:
op_ul2f:
op_pluseqc:
op_pluseq:
op_minuseqc:
op_minuseq:
op_postincc:
op_postinc:
op_postdecc:
op_postdec:
op_muleqc:
op_muleq:
op_diveqc:
op_divequc:
op_diveq:
op_divequ:
op_remeqc:
op_remequc:
op_remeq:
op_remequ:
op_andeqc:
op_andeq:
op_oreqc:
op_oreq:
op_xoreqc:
op_xoreq:
op_shleqc:
op_shleq:
op_shreqc:
op_shrequc:
op_shreq:
op_shrequ:
op_callfname:
	sex SP
	lda BPC
	plo TMP		; target addr lo
	lda BPC
	phi TMP		; target addr hi
	ghi BPC
	stxd		; save old addr
	glo BPC
	stxd
	ghi FP
	stxd
	glo FP
	stxd
	glo TMP
	plo BPC
	ghi TMP
	phi BPC
	sex BPC
	sep RUN
op_callfunc:
	sex SP
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	ghi BPC
	stxd
	glo BPC
	stxd
	ghi FP
	stxd
	glo FP
	stxd
	glo TMP
	plo BPC
	ghi TMP
	phi BPC
	sex BPC
	sep RUN
op_jfalse:
	glo AC
	bz op_jump
bnot:
	inc BPC
	inc BPC
	sep RUN
op_jtrue:
	glo AC
	bz bnot
op_jump:
	lda BPC
	plo TMP
	ldn BPC
	phi BPC
	glo TMP
	plo BPC
	sep RUN
op_switchc:
op_switch:
op_cceq:
	sex SP
	glo AC
	inc SP
	sm
	inc SP
	bnz ccfalse
	ghi AC
	sm
	bnz ccfalse
cctrue:
	ldi 0	; already 0 but this lets us share the stub
	phi AC
	ldi 1
	plo AC
	sex BPC
	sep RUN
ccfalse:
	ldi 0
	plo AC
	phi AC
	sex BPC
	sep RUN
op_cclt:
	; TOS < AC
	sex SP
	inc SP
	inc SP
	ghi AC
	xor
	shl
	bnf lt_sign_same
	shr
	xor
	shl	; get sign into D
	ldi 0
	phi AC
	lsdf
	ldi 1
	plo AC
	sex BPC
	sep RUN
lt_sign_same:
	dec SP
	ghi AC
	sm
	bz cclt_2
	ldi 1
	lsdf
	ldi 0
	inc SP
	sex BPC
	sep RUN
cclt_2:
	inc SP
	glo AC
	sm
	ldi 1
	lsdf
	ldi 0
	plo AC
	sex BPC
	sep RUN
op_ccltu:
	ldi 0
	phi AC
	sex SP
	inc SP
	inc SP		; high byte
	glo AC
	sm
	bz ltu2
	ldi 0
	lsdf
	ldi 1
	plo AC
	sex BPC
	sep RUN
ltu2:
	dec SP
	glo AC
	sm
	inc SP
	ldi 0
	lsdf
	ldi 1
	plo AC
	sex BPC
	sep RUN
op_cclteq:
	; TOS < AC
	sex SP
	inc SP
	inc SP
	ghi AC
	xor
	shl
	bdf lteq_sign_same
	shr
	xor
	shl	; get sign into D
	ldi 0
	phi AC
	lsdf
	ldi 1
	plo AC
	sex BPC
	sep RUN
lteq_sign_same:
	dec SP
	ghi AC
	sm
	bz cclt_2
	ldi 1
	lsdf
	ldi 0
	inc SP
	sex BPC
	sep RUN
cclteq_2:
	inc SP
	glo AC
	sm
	bz setleq
	ldi 1
	lsdf
	ldi 0
	plo AC
	sex BPC
	sep RUN
setleq:
	ldi 1
	plo AC
	sex BPC
	sep RUN
op_ccltequ:
	ldi 0
	phi AC
	sex SP
	inc SP
	inc SP		; high byte
	glo AC
	sm
	bz ltequ2
	ldi 0
	lsdf
	ldi 1
	plo AC
	sex BPC
	sep RUN
ltequ2:
	dec SP
	glo AC
	sm
	inc SP
	bz ltequ3
	ldi 0
	lsdf
ltequ3:
	ldi 1
	plo AC
	sex BPC
	sep RUN
op_nrefc:
	ldxa
	plo TMP
	ldxa
	phi TMP
	lda TMP
	plo AC
	sep RUN
op_nref:
	ldxa
	plo TMP
	ldxa
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
	sep RUN
op_lrefc:
	glo FP
	add
	plo TMP
	irx
	ghi FP
	adc
	phi TMP
	ldn TMP
	plo AC
	sep RUN
op_lref:
	glo FP
	add
	plo TMP
	irx
	ghi FP
	adc
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
	sep RUN
op_nstorec:
	ldxa
	plo TMP
	ldxa
	phi TMP
	glo AC
	str TMP
	sep RUN
op_nstore:
	ldxa
	plo TMP
	ldxa
	phi TMP
	glo AC
	str TMP
	inc TMP
	ghi AC
	str TMP
	sep RUN
op_lstorec:
	glo FP
	add
	plo TMP
	irx
	ghi FP
	adc
	phi TMP
	glo AC
	str TMP
	sep RUN
op_lstore:
	glo FP
	add
	plo TMP
	irx
	ghi FP
	adc
	phi TMP
	glo AC
	str TMP
	inc TMP
	ghi AC
	str TMP
	sep RUN
op_local:
	glo FP
	add
	plo AC
	irx
	ghi FP
	adc
	phi AC
	sep RUN
op_plusconst:
	sex BPC
	glo AC
	add
	plo AC
	ghi AC
	inc BPC
	add
	phi AC
	sep RUN
op_plus4:
	inc AC
op_plus3:
	inc AC
op_plus2:
	inc AC
op_plus1:
	inc AC
	sep RUN
op_minus4:
	dec AC
op_minus3:
	dec AC
op_minus2:
	dec AC
op_minus1:
	dec AC
	sep RUN
op_fnenter:
	glo SP
	add
	plo FP
	plo SP
	inc BPC
	ghi SP
	add
	phi FP
	phi SP
	sep RUN
op_fnexit:
	inc SP
	lda SP
	plo FP
	lda SP
	phi FP
	lda SP
	plo BPC
	ldn SP
	phi BPC
	sep RUN
op_cleanup:
	sex BPC
	glo SP
	add
	plo SP
	inc BPC
	ghi SP
	adc
	phi SP
	inc BPC
	sep RUN
op_native:
	ldxa
	plo TMP
	ldxa
	phi TMP
	sep TMP
	; caller will return by doing sep run and reappearing
	; in bexecute
op_clear:
	ldi 0
	phi AC
	plo AC
	sep RUN
op_byte:
	ldi 0
	phi AC
	sep RUN
op_shift0:
	ldi >byteop
	phi VP
	sep RUN
op_pushl:
	sex SP
	ghi HI
	stxd
	glo HI
	stxd
	ghi AC
	stxd
	glo AC
	stxd
	sex BPC
	sep RUN
op_shrll:
	; We should do smart things for 8bit shift
	glo AC
	ani 15
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
	inc SP
	ldn SP
	phi AC
	shl
	bdf llsrnext
	br asrlnext
asrlloop:
	ghi HI
	shr
	adi 0x80
	phi HI
	glo HI
	shrc
	plo HI
	ghi AC
	shrc
	phi AC
	glo AC
	shrc
	plo AC
asrlnext:
	dec TMP
	glo TMP
	bnz asrlloop
	sep RUN
op_shrul:
	; We should do smart things for 8bit shift
	glo AC
	ani 15
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
	inc SP
	ldn SP
	phi AC
	br llsrnext
llsrloop:
	ghi HI
	shr
	phi HI
	glo HI
	shrc
	plo HI
	ghi AC
	shrc
	phi AC
	glo AC
	shrc
	plo AC
llsrnext:
	dec TMP
	glo TMP
	bnz llsrloop
	sep RUN
op_shll:
	glo AC
	ani 31
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
	lda SP
	phi AC
	lda SP
	plo HI
	ldn SP
	phi HI
	br lsllnext
lsllloop:
	glo AC
	shl
	plo AC
	ghi AC
	shlc
	phi AC
	glo HI
	shlc
	plo HI
	ghi HI
	shlc
	phi HI
lsllnext:
	dec TMP
	glo TMP
	bnz lsllloop
	sep RUN
op_plusf:
op_plusl:
	sex SP
	inc SP
	glo AC
	add
	plo AC
	inc SP
	ghi AC
	adc
	phi AC
	glo HI
	adc
	plo HI
	inc SP
	ghi HI
	adc
	phi HI
	sex BPC
	sep RUN
op_minusf:
op_minusl:
	sex SP
	inc SP
	glo AC
	sd
	plo AC
	inc SP
	ghi AC
	sdb
	phi AC
	inc SP
	glo HI
	sdb
	plo HI
	inc SP
	ghi HI
	sdb
	phi HI
	sex BPC
	sep RUN
op_mulf:
op_mull:
op_divf:
op_divl:
op_divul:
op_remf:
op_reml:
op_remul:
op_negatef:
op_negatel:
	glo AC
	xri 0xFF
	plo AC
	ghi AC
	xri 0xFF
	phi AC
	glo HI
	xri 0xFF
	plo HI
	ghi HI
	xri 0xFF
	phi HI
	inc AC
	glo AC
	bnz negdone
	ghi AC
	bnz negdone
	inc HI
negdone:
	sep RUN
op_bandl:
	sex SP
	inc SP
	glo AC
	and
	plo AC
	inc SP
	ghi AC
	and
	phi AC
	inc SP
	glo HI
	and

	plo HI
	inc SP
	ghi HI
	and
	phi HI
	sex SP
	sep RUN
op_orl:
	sex SP
	inc SP
	glo AC
	or
	plo AC
	inc SP
	ghi AC
	or
	phi AC
	inc SP
	glo HI
	or
	plo HI
	inc SP
	ghi HI
	or
	phi HI
	sex SP
	sep RUN
op_xorl:
	sex SP
	inc SP
	glo AC
	or
	plo AC
	inc SP
	ghi AC
	xor
	phi AC
	inc SP
	glo HI
	xor
	plo HI
	inc SP
	ghi HI
	xor
	phi HI
	sex SP
	sep RUN
op_assignl:
	inc SP
	lda SP
	str AC
	plo TMP
	lda SP
	str AC
	phi AC
	lda SP
	str AC
	plo HI
	ldn SP
	str AC
	phi HI
	glo TMP
	plo AC
	sep RUN
op_derefl:
	lda AC
	plo TMP
	lda AC
	phi TMP
	lda AC
	plo HI
	lda AC
	phi HI
	glo TMP
	plo AC
	ghi TMP
	phi AC
	sep RUN
op_constl:
	ldxa
	plo AC
	ldxa
	phi AC
	ldxa
	plo HI
	ldxa
	phi HI
	sep RUN
op_pluseqf:
op_pluseql:
op_minuseqf:
op_minuseql:
op_postincf:
op_postincl:
op_postdecf:
op_postdecl:
op_muleqf:
op_muleql:
op_diveqf:
op_diveql:
op_divequl:
op_remeqf:
op_remeql:
op_remequl:
op_andeql:
op_oreql:
op_xoreql:
op_shleql:
op_shreql:
op_shrequl:
op_switchl:
op_cceqf:
op_cceql:
op_ccnef:
op_ccnel:
op_ccltf:
op_ccltl:
op_ccltul:
op_cclteqf:
op_cclteql:
op_ccltequl:
op_ccgtf:
op_ccgtl:
op_ccgtul:
op_ccgteqf:
op_ccgteql:
op_ccgtequl:
op_nrefl:
	ldxa
	plo TMP
	ldxa
	phi TMP
	lda TMP
	plo AC
	lda TMP
	phi AC
	lda TMP
	plo HI
	ldn TMP
	phi HI
	sep RUN
op_lrefl:
	glo FP
	add
	plo TMP
	irx
	ghi FP
	adc
	phi TMP
	lda TMP
	plo AC
	lda TMP
	phi AC
	lda TMP
	plo HI
	ldn TMP
	phi HI
	sep RUN
op_nstorel:
	ldxa
	plo TMP
	ldxa
	phi TMP
	glo AC
	str TMP
	inc TMP
	ghi AC
	str TMP
	inc TMP
	glo HI
	str TMP
	inc TMP
	ghi HI
	str TMP
	sep RUN
op_lstorel:
	glo FP
	add
	plo TMP
	irx
	ghi FP
	adc
	phi TMP
	glo AC
	str TMP
	inc TMP
	ghi AC
	str TMP
	inc TMP
	glo HI
	str TMP
	inc TMP
	ghi HI
	str TMP
	sep RUN
op_r0ref:
	ghi R_0
	phi AC
op_r0refc:
	glo R_0
	plo AC
	sep RUN
op_r0store:
	ghi AC
	phi R_0
op_r0storec:
	glo AC
	plo R_0
	sep RUN
op_r0derefc:
	glo R_0
	plo TMP
	ghi R_0
	phi TMP
	ldn TMP
	plo AC
	sep RUN
op_r0drfpre:
	inc R_0
	inc R_0
op_r0deref:
	glo R_0
	plo TMP
	ghi R_0
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
	sep RUN
op_r0inc2:
	inc R_0
op_r0inc1:
	inc R_0
	sep RUN
op_r0dec2:
	dec R_0
op_r0dec:
	dec R_0
	sep RUN
op_r0drfpost:
	lda R_0
	plo AC
	lda R_0
	phi AC
	sep RUN
op_r1ref:
	ghi R_1
	phi AC
op_r1refc:
	glo R_1
	plo AC
	sep RUN
op_r1store:
	ghi AC
	phi R_1
op_r1storec:
	glo AC
	plo R_1
	sep RUN
op_r1derefc:
	glo R_1
	plo TMP
	ghi R_1
	phi TMP
	ldn TMP
	plo AC
	sep RUN
op_r1drfpre:
	inc R_1
	inc R_1
op_r1deref:
	glo R_1
	plo TMP
	ghi R_1
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
	sep RUN
op_r1inc2:
	inc R_1
op_r1inc1:
	inc R_1
	sep RUN
op_r1dec2:
	dec R_1
op_r1dec:
	dec R_1
	sep RUN
op_r1drfpost:
	lda R_1
	plo AC
	lda R_1
	phi AC
	sep RUN
op_r2ref:
	ghi R_2
	phi AC
op_r2refc:
	glo R_2
	plo AC
	sep RUN
op_r2store:
	ghi AC
	phi R_2
op_r2storec:
	glo AC
	plo R_2
	sep RUN
op_r2derefc:
	glo R_2
	plo TMP
	ghi R_2
	phi TMP
	ldn TMP
	plo AC
	sep RUN
op_r2drfpre:
	inc R_2
	inc R_2
op_r2deref:
	glo R_2
	plo TMP
	ghi R_2
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
	sep RUN
op_r2inc2:
	inc R_2
op_r2inc1:
	inc R_2
	sep RUN
op_r2dec2:
	dec R_2
op_r2dec:
	dec R_2
	sep RUN
op_r2drfpost:
	lda R_2
	plo AC
	lda R_2
	phi AC
	sep RUN
op_r3ref:
	ghi R_3
	phi AC
op_r3refc:
	glo R_3
	plo AC
	sep RUN
op_r3store:
	ghi AC
	phi R_3
op_r3storec:
	glo AC
	plo R_3
	sep RUN
op_r3derefc:
	glo R_3
	plo TMP
	ghi R_3
	phi TMP
	ldn TMP
	plo AC
	sep RUN
op_r3drfpre:
	inc R_3
	inc R_3
op_r3deref:
	glo R_3
	plo TMP
	ghi R_3
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
	sep RUN
op_r3inc2:
	inc R_3
op_r3inc1:
	inc R_3
	sep RUN
op_r3dec2:
	dec R_3
op_r3dec:
	dec R_3
	sep RUN
op_r3drfpost:
	lda R_3
	plo AC
	lda R_3
	phi AC
	sep RUN
op_invalid:

; Set up to run bytecode at address in bpc, C stack at sp
;
byteop_init:
	ldi	>byteop
	phi	VP
	ldi	>byteop_run
	phi	RUN
	ldi	<byteop_run
	plo	RUN
	sep	RUN
byteop_run:
	lda	BPC
	plo	VP
	lda	VP
	plo	PC
	ldn	PC
	sep	PC
	br	byteop_run
