;
;		H holds the pointer
;
		.export __derefl
		.setcpu	8085
		.code

__derefl:
		ldhi	2
		lhlx
		shld	__hireg
		dcx	d
		dcx	d
		lhlx
		ret
