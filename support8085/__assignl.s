;
;	Assign the value on top of stack to m. The assigned value also
;	needs to end up in hireg/HL
;
		.export __assignl
		.export	__assign0l
		.setcpu 8085
		.code

__assignl:
		xchg
		pop	h
		shld	__retaddr
		pop	h
		shld	__tmp
		shlx
		inx	d
		inx	d
		pop	h
		shlx
		shld	__hireg
		lhld	__tmp
		jmp	__ret

__assign0l:
		xchg		; address into d
		lxi	h,0
		shlx
		shld	__hireg
		inx	d
		inx	d
		shlx
		ret
