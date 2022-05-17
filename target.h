#define TARGET_MAX_INT		32767L
#define TARGET_MAX_LONG		2147483647UL	/* and a double persenne prime too */
#define TARGET_MAX_UINT		65535UL

extern unsigned target_sizeof(unsigned t);
extern unsigned target_alignof(unsigned t);
extern unsigned target_argsize(unsigned t);
extern unsigned target_ptr_arith(unsigned t);
extern unsigned target_scale_ptr(unsigned t, unsigned scale);

/* Default integer type is 2 byte */
#define CINT	CSHORT
#define UINT	USHORT
