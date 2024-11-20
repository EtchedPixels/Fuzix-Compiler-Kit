;
;	Function entry register saves
;
	.export __rpush2
	.export __rpush1

__rpush2:
	mov r7,a
	sta *r15
	decd r15
	mov r6,a
	sta *r15
	decd r15
__rpush1:
	mov r9,a
	sta *r15
	decd r15
	mov r8,a
	sta *r15
	decd r15
	rets
