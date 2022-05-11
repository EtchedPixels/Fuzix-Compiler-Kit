
/* Pass 2 values */
#define MAXSYM		512

#include <stdio.h>

#include "symbol.h"

#include "body.h"
#include "declaration.h"
#include "error.h"
#include "expression.h"
#include "header.h"
#include "idxdata.h"
#include "initializer.h"
#include "label.h"
#include "lex.h"
#include "primary.h"
#include "storage.h"
#include "stackframe.h"
#include "struct.h"
#include "switch.h"
#include "target.h"
#include "token.h"
#include "tree.h"
#include "type.h"
#include "type_iterator.h"

extern FILE *debug;
