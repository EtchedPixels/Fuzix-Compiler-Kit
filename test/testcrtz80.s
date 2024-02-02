;
;	Stub crt for running tests
;
	call	_main
	ld	a,l
	out	(0xFF),a
