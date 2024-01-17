;
;		H holds the pointer
;
		.export __dereff
		.export __dereffsp
		.export __derefl
		.export __dereflsp
		.setcpu	8085
		.code

__dereffsp:
__dereflsp:
		dad	sp
__dereff:
__derefl:
		ldhi	2
		lhlx
		shld	__hireg
		dcx	d
		dcx	d
		lhlx
		ret
