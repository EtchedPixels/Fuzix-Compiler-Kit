/*
 *	Headers. These blocks carry the structure of the program outside of
 *	the expressions
 */


#include <unistd.h>
#include "header.h"

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
