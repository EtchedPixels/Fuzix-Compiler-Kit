#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

#define ARGBASE	2	/* Bytes between arguments and locals if no reg saves */

#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)
#define T_DEREFPLUS	(T_USER+7)
#define T_EQPLUS	(T_USER+8)
#define T_LDEREF	(T_USER+9)	/* *local + offset */
#define T_LEQ		(T_USER+10)	/* *local + offset = n */
#define T_NDEREF	(T_USER+11)	/* *(name + offset) */
#define T_LBDEREF	(T_USER+12)	/* *(name + offset) */
#define T_NEQ		(T_USER+13)	/* *(name + offset) = n */
#define T_LBEQ		(T_USER+14)	/* *(label + offset) = n */
#define T_RREF		(T_USER+15)
#define T_RSTORE	(T_USER+16)
#define T_RDEREF	(T_USER+17)		/* *regptr */
#define T_REQ		(T_USER+18)		/* *regptr = */
#define T_RDEREFPLUS	(T_USER+19)		/* *regptr++ */
#define T_REQPLUS	(T_USER+20)		/* *regptr++ =  */

extern unsigned frame_len;	/* Number of bytes of stack frame */
extern unsigned sp;		/* Stack pointer offset tracking */
extern unsigned argbase;	/* Argument offset in current function */

extern unsigned get_size(unsigned t);
extern unsigned get_stack_size(unsigned t);

extern void repeated_op(unsigned n, const char *op);
extern const char *remap_op(const char *op);

extern unsigned cpu_has_d;	/* 16bit ops and 'D' are present */
extern unsigned cpu_has_xgdx;	/* XGDX is present */
extern unsigned cpu_has_abx;	/* ABX is present */
extern unsigned cpu_has_pshx;	/* Has PSHX PULX */
extern unsigned cpu_has_y;	/* Has Y register */
extern unsigned cpu_has_lea;	/* Has LEA. For now 6809 but if we get to HC12... */
extern unsigned cpu_is_09;	/* Bulding for 6x09 so a bit different */
extern unsigned cpu_pic;	/* Position independent output (6809 only) */
extern const char *ld8_op;
extern const char *st8_op;
extern const char *pic_op;

/* Tracking */
extern uint8_t a_val;
extern uint8_t b_val;
extern unsigned a_valid;
extern unsigned b_valid;
extern unsigned d_valid;
extern uint16_t x_fpoff;
extern unsigned x_fprel;

extern void invalidate_all(void);
extern void invalidate_x(void);
extern void invalidate_a(void);
extern void invalidate_b(void);
extern void invalidate_d(void);
extern void invalidate_work(void);
extern void invalidate_mem(void);
extern void set_d_node(struct node *n);
extern void set_d_node_ptr(struct node *n);
extern unsigned d_holds_node(struct node *n);
extern void modify_a(uint8_t val);
extern void modify_b(uint8_t val);

/* Code generation helpers */
extern void load_d_const(uint16_t n);
extern void load_a_const(uint8_t n);
extern void load_b_const(uint8_t n);
extern void add_d_const(uint16_t n);
extern void add_b_const(uint8_t n);
extern void load_a_b(void);
extern void load_b_a(void);
extern void move_s_d(void);
extern void move_d_s(void);
extern void swap_d_y(void);
extern void swap_d_x(void);
extern void make_x_d(void);
extern void pop_x(void);
extern void adjust_s(int n, unsigned save_d);
extern void op8_on_ptr(const char *op, unsigned off);
extern void op16_on_ptr(const char *op, const char *op2, unsigned off);
extern void op16d_on_ptr(const char *op, const char *op2, unsigned off);
extern void op8_on_s(const char *op, unsigned off);
extern void op8_on_spi(const char *op);
extern void op16_on_s(const char *op, const char *op2, unsigned off);
extern void op16d_on_spi(const char *op);
extern void op16_on_spi(const char *op, const char *op2);
extern void op16d_on_s(const char *op, const char *op2, unsigned off);
extern void op32_on_ptr(const char *op, const char *op2, unsigned off);
extern void op32d_on_ptr(const char *op, const char *op2, unsigned off);
extern unsigned make_local_ptr(unsigned off, unsigned rlim);
extern unsigned make_tos_ptr(void);
extern unsigned op8_on_node(struct node *r, const char *op, unsigned off);
extern unsigned op16_on_node(struct node *r, const char *op, const char *op2, unsigned off);
extern unsigned op16d_on_node(struct node *r, const char *op, const char *op2, unsigned off);
extern unsigned op16y_on_node(struct node *r, const char *op, unsigned off);
extern unsigned write_op(struct node *r, const char *op, const char *op2, unsigned off);
extern unsigned write_opd(struct node *r, const char *op, const char *op2, unsigned off);
extern unsigned write_uni_op(struct node *n, const char *op, unsigned off);
extern void uniop_on_ptr(const char *op, unsigned off, unsigned size);
extern void op8_on_tos(const char *op);
extern void op16_on_tos(const char *op, const char *op2);
extern void op16d_on_tos(const char *op, const char *op2);
extern unsigned write_tos_op(struct node *n, const char *op, const char *op2);
extern void uniop8_on_tos(const char *op);
extern void uniop16_on_tos(const char *op);
extern unsigned write_tos_uniop(struct node *n, const char *op);
extern unsigned left_shift(struct node *n);
extern unsigned right_shift(struct node *n);
extern unsigned can_load_r_simple(struct node *r, unsigned off);
extern unsigned can_load_r_with(struct node *r, unsigned off);
extern unsigned load_x_with(struct node *r, unsigned off);
extern unsigned load_u_with(struct node *r, unsigned off);
