/*
 *	Headers. These blocks carry the structure of the program outside of
 *	the expressions
 */


#include <unistd.h>
#include <stdlib.h>
#include "compiler.h"

void header(unsigned htype, unsigned name, unsigned data)
{
	struct header h;
	h.h_type = htype;
	h.h_name = name;
	h.h_data = data;

	write(1, "%H", 2);	/* For now until it's all headers/expr */
	write(1, &h, sizeof(h));
}

void footer(unsigned htype, unsigned name, unsigned data)
{
	header(htype | H_FOOTER, name, data);
}

void rewrite_header(off_t pos, unsigned htype, unsigned name, unsigned data)
{
	if (lseek(1, pos, SEEK_SET) == -1) {
		perror("hseek");
		exit(1);
	}
	header(htype, name, data);
	if (lseek(1, 0L, SEEK_END) == -1) {
		perror("hseek");
		exit(1);
	}
}

off_t mark_header(void)
{
	return lseek(1, 0L, SEEK_CUR);
}
