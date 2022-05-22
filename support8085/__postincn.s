		.export __postinc1
		.export __postinc2

		.setcpu 8085
		.code

__postinc1:
		xchg
		lhlx
		inx	h
		shlx
		dcx	h
		ret
__postinc2:
		xchg
		lhlx
		inx	h
		inx	h
		shlx
		dcx	h
		dcx	h
		ret


