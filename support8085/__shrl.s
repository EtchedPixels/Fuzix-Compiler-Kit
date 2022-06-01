		.export __shrl
		.setcpu 8080
		.code

__shrl:
		mov	a,l		; shift amount
		pop	h
		shld	__retaddr
		pop	d
		pop	h		; shifting HLDE by A
		ani	31		; nothing to do ?
		jz	done

		push	psw

		mvi	b,0
		mov	a,h
		ora	a
		jp	zerofill
		dcr	b
zerofill:
		pop	psw
;
;	Shortcut, do the bytes by register swap
;
		cpi	24
		jc	not3byte
		mov	e,h
		mov	h,b
		mov	l,b
		mov	d,b
		sui	24
		jmp	leftover

not3byte:
		cpi	16
		jc	not2byte
		xchg			; HL into DE
		mov	h,b
		mov	l,b
		sui	16
		jmp	leftover
not2byte:
		cpi	8
		jc	leftover
		mov	e,d
		mov	d,l
		mov	l,h
		mov	h,b
		sui	8
;
;	Do any remaining work
;
leftover:
		jz	done
		push	b
		mov	c,a		; count into C
shloop:
		mov	a,h
		add	a		; will set carry if top bit was set
		rar			; shifts in the carry
		mov	h,a
		mov	a,l
		rar
		mov	l,a
		mov	a,d
		rar
		mov	d,a
		mov	a,e
		rar
		mov	e,a
		dcr	c
		jnz	shloop
		pop	b
done:
		shld	__hireg
		xchg
		jmp	__ret
