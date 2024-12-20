;
;	r12.r13 is the struct offset, r11 is the field offset
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
	add r15,r13		; add in SP
	adc r14,r12
	lda *r13		; get local low byte
	decd r13
	lda *r13
	add b,r11		; add the offset
	adc %0,r10
	rets

__lstref0:
	clr r12
__lstref:
	call @dolst
	lda *r13
	mov a,r5
	decd r13
	lda *r13
	mov a,r4
	rets

__lstref0c:
	clr r12
__lstrefc:
	call @dolst
	lda *r13
	mov a,r5
	rets

__lstref0l:
	clr r12
__lstrefl:
	call @dolst
	lda *r13
	mov a,r5
	decd r13
	lda *r13
	mov a,r4
	decd r13
	lda *r13
	mov a,r3
	decd r13
	lda *r13
	mov a,r2
	rets

__lststore0:
	clr r12
__lststore:
	call @dolst
	mov r5,a
	sta *r13
	decd r13
	mov r4,a
	sta *r13
	rets

__lststore0c:
	clr r12
__lststorec:
	call @dolst
	mov r5,a
	sta *r13
	rets

__lststore0l:
	clr r12
__lststorel:
	call @dolst
	mov r5,a
	sta *r13
	decd r13
	mov r4,a
	sta *r13
	decd r13
	mov r3,a
	sta *r13
	decd r13
	mov r2,a
	sta *r13
	rets
