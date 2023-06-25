		.export __bcdiv
		.export __bcdivu
		.export __bcdivc
		.export __bcdivuc

		; We compute HL/DE but righh now we have BC / HL
__bcdivuc:
		mvi d,0
		mov b,d
__bcdivu:
		xchg
		mov e,c
		mov d,b
		call __divdeu
		mov c,l
		mov h,b
		ret

__bcmoduc:
		mvi d,0
		mov b,d
__bcmodu:
		xchg
		mov e,c
		mov d,b
		call __divdeu
		xchg
		mov c,l
		mov h,b
		ret
