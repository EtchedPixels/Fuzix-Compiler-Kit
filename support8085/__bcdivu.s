		.export __bcdivu
		.export __bcremu

		; We compute HL/DE but righh now we have BC / HL
__bcdivu:
		xchg
		mov e,c
		mov d,b
		call __divdeu
		mov c,l
		mov b,h
		ret

__bcremu:
		xchg
		mov e,c
		mov d,b
		call __divdeu
		xchg
		mov c,l
		mov b,h
		ret
