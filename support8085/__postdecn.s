		.export __postdec1
		.export __postdec2

		.setcpu 8085
		.code

__postdec1:
		xchg
		lhlx
		dcx	h
		shlx
		inx	h
		ret
__postdec2:
		xchg
		lhlx
		dcx	h
		dcx	h
		shlx
		inx	h
		inx	h
		ret


