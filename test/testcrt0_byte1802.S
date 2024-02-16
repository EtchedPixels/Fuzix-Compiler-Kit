#include "../support1802/1802ops.h"
	.code
start:
	.byte	op_callfname		; callfname
	.word	_main
	.byte	op_native		; traps back out
