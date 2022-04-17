/*
 *	Headers. These blocks carry the structure of the program outside of
 *	the expressions
 */


struct header {
    unsigned h_type;
    unsigned h_name;
    unsigned h_data;
};

#define H_FOOTER	0x8000

#define H_FUNCTION	0x0001
/* For these we need to figure out who owns stack offsetting and name types */
#define H_LOCAL		0x0002	/* local var name size */
#define H_LOCALASSIGN	0x0003	/* local var name size <expr> */
#define	H_ARGUMENT	0x0004	/* argument name - */
#define H_IF		0x0005	/* if */
#define H_ELSE		0x0006	/* else */
#define H_WHILE		0x0007	/* while */
#define H_DO		0x0008	/* start of do-while */
#define H_DOWHILE	0x0009	/* do .. while expr */
#define H_FOR		0x000A	/* for */
#define H_SWITCH	0x000B	/* switch */
#define H_CASE		0x000C	/* case */
#define H_DEFAULT	0x000D	/* default */
#define H_BREAK		0x000E	/* break */
#define H_CONTINUE	0x000F	/* continue */
#define H_RETURN	0x0010	/* return */
#define H_LABEL		0x0011	/* goto label */
#define H_GOTO		0x0012	/* goto */
#define H_STRING	0x0013	/* string */
#define H_FRAME		0x0014	/* declare the stack frame */

extern void header(unsigned htype, unsigned name, unsigned data);
extern void footer(unsigned htype, unsigned name, unsigned data);
extern void rewrite_header(off_t off, unsigned htype, unsigned name, unsigned data);
extern off_t mark_header(void);
