;
;		(TOS) /= HL
;

			.export __modequ
			.setcpu 8085
			.code
__modequ:
	xchg
	pop	h
	xthl
	; Now we are doing (HL) * DE
	push	d
	xchg
	lhlx	; get TOS into HL
	xchg
	xthl	; swap address with stacked value
	xchg	; swap them back as we modide by DE
	; We are now doing HL / DE and the address we want is TOS
	call __moddeu
	; Return is in HL
	pop	d
	shlx
	ret
