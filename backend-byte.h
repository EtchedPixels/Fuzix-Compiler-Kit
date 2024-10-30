/*
 *	Options for byte mode. Uses the first half of the user flag bits
 */

#define BYTEABLE	0x0100		/* Candidate for size reduction */
#define BYTEOP		0x0200		/* Do size reduced */
#define BYTEROOT	0x0400		/* Start of a reduced section */
#define BYTETAIL	0x0800		/* End of a reduced section */

extern void byte_label_tree(struct node *n, unsigned flags);

#define BTF_RELABEL	0x0001		/* Relabel sizes where possible */

