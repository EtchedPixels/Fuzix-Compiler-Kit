	.65c816
	.a16
	.i16

	.export __cast_l

	; TODO : inline ?

__cast_l:
	; cast A into hireg:A
	stz @hireg
	tax	; just to get the n flag set right
	beq done
	dec @hireg
done:	rts
