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

;		0	; DMA
;		1	; Interrupt handler
;		2	; System stack for interrupt
#define AC	3	; Accumulator low
#define HI	4	; Accumulator high
#define SP	5	; Thread code stack (must bump on 4 bytes before
			; reusing in interrupts)
#define BPC	6	; Threadcode program counter
#define PC	7	; Program counter for the thread op
#define RUN	8	; Program counter for the main co-routine
#define TMP	9	; Working scratch
#define FP	10	; Frame pointer	
			; 11-15 free (register vars perhaps one day ?)


	.export __execute

;	Run threadcode ops. We fetch an address from bpc and execute it as a
;	couroutine in pc. run holds the execute co-routine address
__execute:
	lda BPC
	plo PC
	lda BPC
	phi PC
	sep PC
	br __execute

;
;	Stack the accumulator byte, word, long sized
;
	.export __pushc
__pushc:
	glo AC
	str SP
	dec SP
	sep RUN

	.export __push
__push:
	sex SP
	ghi AC
	stxd
	glo AC
	stxd
	sex BPC
	sep RUN

	.export __pushl

__pushl:
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

;
;	16bit unsigned right shift of top of stack by AC.lo
;
	.export __shru
__shru:			; needs smarts for 8bit
	glo AC
	ani 15
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
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

;
;	Signed version of above
;
	.export __shr
__shr:
	glo AC
	ani 15
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
	ldn SP
	phi AC
	shl
	bdf lsrnext
	br asrnext
asrloop:	; negative loop
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

;
;	16bit signed/unsigned left shift of the top of stack by AC.lo
;
	.export __shl
__shl:			; needs smarts for 8bit
	glo AC
	ani 15
	plo TMP
	inc TMP
	inc SP
	lda SP
	plo AC
	ldn SP
	phi AC
	br lslnext
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

;
;	32bit version
;
	.export __shll
__shll:			; needs smarts for 8bit and 16bit breaks
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

;
;	16 and 32bit addition between top of stack and AC/HI
;	May well be worth having optimized ops for constant later
;
	.export __plus
__plus:
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

	.export __plusl
__plusl:
	sex SP
	inc SP
	glo AC
	add
	plo AC
	inc SP	
	ghi AC
	adc
	phi AC
	inc SP
	glo HI
	adc
	plo HI
	inc SP	
	ghi HI
	adc
	phi HI
	sex BPC
	sep RUN

	.export __minus
;	TOS - AC
__minus:
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

	.export __minusl
__minusl:
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

;
;	Boolean logic between top of stack and AC or AC/HI
;
	.export __band
__band:
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

	.export __bandl
__bandl:
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


	.export __xor
__xor:
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

	.export __xorl
__xorl:
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

	.export __or
__or:
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

	.export __orl
__orl:
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

;
;	Complement AC or AC/HI
;
	.export __cpll
	.export __cpl
__cpll:
	glo HI
	xri 0xFF
	plo HI
	ghi HI
	xri 0xFF
	phi HI
__cpl:	glo AC
	xri 0xFF
	plo AC
	ghi AC
	xri 0xFF
	plo AC
	sep RUN

;
;	Assignment (all sizes). AC into (TOS)
;	

	; put TOS into (AC)
	.export __eqc
__eqc:
	inc SP
	ldn SP
	str AC
	plo AC		; stored value becomes working
	sep RUN

	.export __eq
__eq:
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

	.export __eql
__eql:
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

;
;	Object dereference (all sizes) AC = (AC)
;
	.export __derefc
	.export __deref
	.export __derefl

__deref:
	lda AC
	plo TMP
	lda AC
	phi AC
	glo TMP
	plo AC
	sep RUN
__derefc:
	lda SP
	plo AC
	sep RUN
__derefl:
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

;
;	Negate AC
;
	.export __negate
	.export __negatel
__negate:
	glo AC
	xri 0xFF
	plo AC
	ghi AC
	xri 0xFF
	phi AC
	inc AC
	sep RUN

__negatel:
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

;
;	Load a constant value into AC
;
	.export __const
	.export __loadn
	.export __loadl

__const:
__loadn:
	ldxa
	plo AC
	ldxa
	phi AC
	sep RUN

	.export __loadl
__loadl:
	ldxa
	plo AC
	ldxa
	phi AC
	ldxa
	plo HI
	ldxa
	phi HI
	sep RUN

;
;	Drop a byte word or long from the stack
;
	.export __popl
	.export __pop
	.export __popc

__popl:
	inc SP
	inc SP
__pop:
	inc SP
__popc:
	inc SP
	sep RUN

;
;	boolean negate AC
;
	.export __notl
	.export __not
	.export __notc

__notl:
	glo HI
	bnz set0
	ghi HI
	bnz set0
__not:
	ghi AC
	bnz set0
__notc:
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
;	Turn AC boolean (0/1)
;
	.export __booll
	.export __bool
	.export __boolc

__booll:
	ghi HI
	bnz set1
	glo HI
	bnz set1
__bool:
	ghi AC
	bnz set1
__boolc:
	glo AC
	lsz
set1:
	ldi 1
	plo AC
	ldi 0
	phi AC
	sep RUN

;	loadl		; adjust into a single operator in core code
;	loada		; we need a tmp as we can't touch SP whilst
;			; working on it

	.export __local
__local:
	sex SP
	glo FP
	inc SP
	add
	plo AC
	ghi FP
	adc
	phi AC
	sex BPC
	sep RUN

;
;	Typecasting of HI:AC as needed
;
	.export __castc_
	.export __castc_l
	.export __castuc_
	.export __castuc_l
	.export __cast_l
	.export __castu_l

__castc_l1:
	ldi 255
	lskp
__castuc_l:
	ldi 0
	phi HI
	plo HI
	lskp
__castuc_:
	ldi 0
	phi AC
	sep RUN

__castc_l:
	glo AC
	shl
	bdf __castc_l1
	br __castuc_l
__castc_:
	glo AC
	shl
	bdf __castuc_
	ldi 255
	phi AC
	sep RUN

__cast_l:
	ghi AC
	shl
	ldi 0xFF
	lsdf
__castu_l:
	ldi 0
	plo HI
	phi HI
	sep RUN

;
;	C "+="
;	TOS = address, A is amount
;
	.export __pluseqc
	.export __pluseq
	.export __pluseql

__pluseqc:
	; AC = value
	; TOS = ptr
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	add
	plo AC
	str TMP
	sex BPC
	sep RUN

__pluseq:
	; AC = value
	; TOS = ptr
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	add
	plo AC
	stxd
	ghi AC
	adc
	phi AC
	str TMP
	sex BPC
	sep RUN

__pluseql:
	; AC = value
	; TOS = ptr
	sex SP
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	dec SP
	sex TMP
	glo AC
	add
	plo AC
	stxd
	ghi AC
	adc
	phi AC
	stxd
	glo HI
	adc
	plo HI
	stxd
	ghi HI
	adc
	phi HI
	str TMP
	sex BPC
	sep RUN

	.export __minuseqc
	.export __minuseq
	.export __minuseql

;
;	C "-="
;
__minuseqc:
	; AC = value
	; TOS = ptr
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	sdb
	plo AC
	str TMP
	sex BPC
	sep RUN

__minuseq:
	; AC = value
	; TOS = ptr
	sex SP
	inc SP

	phi TMP
	sex TMP
	glo AC
	sdb
	plo AC
	stxd
	ghi AC
	sdb
	phi AC
	str TMP
	sex BPC
	sep RUN

__minuseql:
	; AC = value
	; TOS = ptr
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	sd
	plo AC
	stxd
	ghi AC
	sdb
	phi AC
	stxd
	glo HI
	sdb
	plo HI
	stxd
	ghi HI
	sdb
	phi HI
	str TMP
	sex BPC
	sep RUN

;
;	Same for logic ops
;
	.export __andeqc
 	.export __andeq
	.export __andeql
__andeqc:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	and
	plo AC
	sex BPC
	sep RUN


__andeq:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	and
	plo AC
	inc TMP
	ghi AC
	and
	phi AC
	sex BPC
	sep RUN

__andeql:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	and
	plo AC
	inc TMP
	ghi AC
	and
	phi AC
	inc TMP
	glo HI
	and
	plo HI
	inc TMP
	ghi HI
	and
	phi HI
	sex BPC
	sep RUN

	.export __oreqc
 	.export __oreq
	.export __oreql
__oreqc:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	or
	plo AC
	sex BPC
	sep RUN

__oreq:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	or
	plo AC
	inc TMP
	ghi AC
	or
	phi AC
	sex BPC
	sep RUN

__oreql:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	or
	plo AC
	inc TMP
	ghi AC
	or
	phi AC
	inc TMP
	glo HI
	or
	plo HI
	inc TMP
	ghi HI
	or
	phi HI
	sex BPC
	sep RUN

	.export __xoreqc
 	.export __xoreq
	.export __xoreql
__xoreqc:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	xor
	plo AC
	sex BPC
	sep RUN

__xoreq:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	xor
	plo AC
	inc TMP
	ghi AC
	xor
	phi AC
	sex BPC
	sep RUN

__xoreql:
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	sex TMP
	glo AC
	xor
	plo AC
	inc TMP
	ghi AC
	xor
	phi AC
	inc TMP
	glo HI
	xor
	plo HI
	inc TMP
	ghi HI
	xor
	phi HI
	sex BPC
	sep RUN

;
;	Call a function. Saves BPC and FP
;
	.export __callfn
__callfn:
	sex SP
	lda BPC
	plo TMP
	lda BPC
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

;
;	Call function whose address is TOS
;
	.export __callfunc

__callfunc:
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

;
;	Function entry helper, builds stack frame and sets FP
;
	.export __fnenter
__fnenter:
	glo SP
	add
	plo FP
	plo SP
	inc BPC
	ghi SP
	adc
	phi FP
	phi SP
	sep RUN

;
;	Return from a function. Restores BPC and FP
;
	.export __fnexit
__fnexit:
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

;
;	Adjust the stack pointer to clean up after a function call
;
	.export __cleanup
__cleanup:
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

;
;	Branches. Use the boolean in AC.lo to decide what happens
;
	.export __jfalse
	.export __jtrue
	.export __jump
__jfalse:
	glo AC
	bz __jump
bnot:
	inc BPC
	inc BPC
	sep RUN
__jtrue:
	glo AC
	bz bnot
__jump:
	lda BPC
	plo TMP
	ldn BPC
	phi BPC
	glo TMP
	plo BPC
	sep RUN

;TODO	shleql

;
;	<<= 	(TOS) <<= AC.lo
;
	.export __shleqc
	.export __shleq
__shleqc:	; shift (TOS) by AC
	glo AC
	ani 15
	bz shleq_nonec
	plo HI
	inc SP
	ldn SP
	plo TMP
	ldn TMP
	plo AC
shleqlpc:
	glo AC
	shl
	plo AC
	dec HI
	glo HI
	bnz shleqlpc
	; AC is now the value, store
	glo AC
	str TMP
	sep RUN

shleq_nonec:
	ldn TMP
	plo AC
	sep RUN

__shleq:	; shift (TOS) by AC
	glo AC
	ani 15
	bz shleq_none
	plo HI
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
shleqlp:
	glo AC
	shl
	plo AC
	ghi AC
	shlc
	phi AC
	dec HI
	glo HI
	bnz shleqlp
	; AC is now the value, store
	ghi AC
	str TMP
	dec TMP
	glo AC
	str TMP
	sep RUN

shleq_none:
	lda TMP
	plo AC
	ldn TMP
	phi AC
	sep RUN

	.export __shrequc
	.export __shrequ

__shrequc:; shift (TOS) by AC
	glo AC
	ani 15
	bz shleq_none
	plo HI
	inc SP
	ldn SP
	plo TMP
	ldn TMP
	plo AC
shreqlpc:
	glo AC
	shr
	plo AC
	dec HI
	glo HI
	bnz shreqlpc
	; AC is now the value, store
	glo AC
	str TMP
	sep RUN

;
;	(TOS) >>= AC.lo
;
__shrequ:	; shift (TOS) by AC
	glo AC
	ani 15
	bz shleq_none
	plo HI
	inc SP
	lda SP
	plo TMP
	ldn SP
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
shreqlp:
	ghi AC
	shr
	phi AC
	glo AC
	shrc
	plo AC
	dec HI
	glo HI
	bnz shreqlp
	; AC is now the value, store
	ghi AC
	str TMP
	dec TMP
	glo AC
	str TMP
	sep RUN

; TODO shreql and signed ver
;	shreq (cl)(u)

;
;	16x16 multiply
;
	.export __mul
__mul:
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


;mull:	/* TODO */
;	mul(l)
;
;	div(l)(s)
;
;	rem(l)(s)
;
;	diveq(cl)(u)
;	muleq(cl)
;	remeq(cl)(u)
;	postinc (cl)	; rewrite as stash pluseq unstash
;		(stash storing to tmp and unstash recovering ?)
;	postdec (cl)



;	cclt(l)(s)
;	ccgt(l)(s)
;	cclteq(l)(s)
;	ccgteq(l)(s)
;	cceq (l)

;
;	Comparisons of TOS and AC. Boolean into AC as result
;
	.export __cceq
__cceq:
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

; ccne: 	; rewrite branch direction on cceq

; for both rewrite >= to ltu reverse conditions,  ditto gt/lteq
	.export __ccltu
__ccltu:
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

	.export __ccltequ
__ccltequ:
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

	.export __cclt
__cclt:
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

	.export __cclteq
__cclteq:
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

	.export __lbrefc
	.export __nrefc
	.export __lrefc
	.export __lbref
	.export __nref
	.export __lref
	.export __nstorec
	.export __nstore
	.export __lbstorec
	.export __lbstore
	.export __lstorec
	.export __lstore

__nrefc:
__lbrefc:
	ldxa
	plo TMP
	ldxa
	phi TMP
	lda TMP
	plo AC
	sep RUN

__nref:
__lbref:
	ldxa
	plo TMP
	ldxa
	phi TMP
	lda TMP
	plo AC
	ldn TMP
	phi AC
	sep RUN

__nstorec:
__lbstorec:
	ldxa
	plo TMP
	ldxa
	phi TMP
	glo AC
	str TMP
	sep RUN

__nstore:
__lbstore:
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

; Could do refl stuff easily enough

__lrefc:
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

__lref:
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
	plo AC
	sep RUN

__lstorec:
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

__lstore:
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

; Optimizer rule generated heleprs

	.export __plus4
	.export __plus3
	.export __plus2
	.export __plus1
	.export __minus4
	.export __minus3
	.export __minus2
	.export __minus1

__plus4:
	inc AC
__plus3:
	inc AC
__plus2:
	inc AC
__plus1:
	inc AC
	sep RUN

__minus4:
	dec AC
__minus3:
	dec AC
__minus2:
	dec AC
__minus1:
	dec AC
	sep RUN

;
;	Now the fun stuff - 16bit divide
;
__rem:
	sex SP
	ghi AC
	shl
	lbz __remu	; Remainder sign matches divisor
			; so low means same as unsigned
	ldi 0x80	; Remember to invert the sign
	lbr negates
__div:
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

__remu:
	ldi 0
	lskp
__divu:
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
