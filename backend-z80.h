/* Until we do more testing */
#define IS_EZ80		0	/* EZ80 has ld de,(hl) and friends but not offset */
#define IS_RABBIT	0	/* Has ld hl,(rr + n) and vice versa but only 16bit */
#define HAS_LDHLSP	0	/* Can ld hl,(sp + n) and vice versa */
#define HAS_LDASP	0	/* Can ld a,(sp + n) and vice versa */
#define HAS_LDHLHL	0	/* Can ld hl,(hl) or hl,(hl + 0) */

#define ARGBASE	2	/* Bytes between arguments and locals if no reg saves */

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)
/*
 *	Upper node flag fields are ours
 */

#define USECC	0x0100

#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)
#define T_RREF		(T_USER+7)
#define T_RSTORE	(T_USER+8)
#define T_RDEREF	(T_USER+9)		/* *regptr */
#define T_REQ		(T_USER+10)		/* *regptr */
#define T_BTST		(T_USER+11)		/* Use bit n, for and bit conditionals */
#define T_BYTEEQ	(T_USER+12)		/* Until we handle 8bit better */
#define T_BYTENE	(T_USER+13)


extern unsigned get_size(unsigned t);
extern unsigned get_stack_size(unsigned t);
extern int bitcheckb1(uint8_t n);
extern int bitcheck1(unsigned n, unsigned s);
extern int bitcheck0(unsigned n, unsigned s);
extern void gen_cleanup(unsigned v);

extern unsigned frame_len;	/* Number of bytes of stack frame */
extern unsigned sp;		/* Stack pointer offset tracking */
extern unsigned argbase;	/* Argument offset in current function */
extern unsigned unreachable;	/* Code after an unconditional jump */
extern unsigned func_cleanup;	/* Zero if we can just ret out */
extern unsigned use_fp;		/* Using a frame pointer this function */

extern const char ccnormal[];
extern const char ccinvert[];

extern const char *ccflags;	/* True, False flags */
