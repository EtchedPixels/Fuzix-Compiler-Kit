;
;	r14.r15 is the struct offset, r13 is the field offset
;
	.export __lstrefc
	.export __lstref
	.export __lstrefl
	.export __lststorec
	.export __lststore
	.export __lststorel
	.export __lstref0c
	.export __lstref0
	.export __lstref0l
	.export __lststore0c
	.export __lststore0
	.export __lststore0l

dolst:
	add r15,217
	adc r14,216		; now a pointer
	push r13		; save the offset to shift the ptr by for a minute
	ldei r12,@rr14
	lde r13,@rr14		; load the local
	pop r15
	add r13,r15		; add the offset
	adc r12,#0
	ret

__lstref0:
	clr r14
__lstref:
	call dolst
	ldei r2,@rr12
	lde r3,@rr12
	ret

__lstref0c:
	clr r14
__lstrefc:
	call dolst
	lde r3,@rr12
	ret

__lstref0l:
	clr r14
__lstrefl:
	call dolst
	ldei r0,@rr12
	ldei r1,@rr12
	ldei r2,@rr12
	ldei r3,@rr12
	ret

__lststore0:
	clr r14
__lststore:
	call dolst
	lde @rr12,r2
	ldepi @rr12,r3
	ret

__lststore0c:
	clr r14
__lststorec:
	call dolst
	lde @rr12,r3
	ret

__lststore0l:
	clr r14
__lststorel:
	call dolst
	lde @rr12,r0
	ldepi @rr12,r1
	ldepi @rr12,r2
	ldepi @rr12,r3
	ret
