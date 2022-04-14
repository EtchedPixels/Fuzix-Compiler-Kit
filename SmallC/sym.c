/*
 * File sym.c: 2.1 (83/03/20,16:02:19)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

/**
 * declare a static variable
 * @param type
 * @param storage
 * @param mtag tag of struct whose members are being declared, or zero
 * @param otag tag of struct object being declared. only matters if mtag is non-zero
 * @param is_struct struct or union or no meaning
 * @return 1 if a function was parsed
 */
int declare_global(unsigned type, int storage, TAG_SYMBOL *mtag, int otag, int is_struct) {
    int     dim;
    unsigned    sname;

    FOREVER {
        FOREVER {
            if (endst ())
                return 0;
            dim = 1;
            /* HACK FIXME */
            if (match (T_STAR))
                type++;
            if (!(sname = symname()))
                illname ();
            if (match (T_LPAREN)) {
                newfunc_typed(storage, sname, type);
                /* Can't int foo(x){blah),a=4; */
                return 1;
            }
            /* FIXME: we need to deal with extern properly here */
            if (find_global (sname) > -1)
                multidef (sname);
            if (type == VOID)
                notvoid(type);
            if (match (T_LSQUARE)) {
                dim = needsub ();
                //if (dim || storage == EXTERN) {
               //                 identity = ARRAY;
                /* Disallow void array FIXME */
                //} else {
                //    identity = POINTER;
                //}
            }
            // add symbol
            if (mtag == 0) { // real variable, not a struct/union member
                type = initials(sname, type, dim, otag);
                add_global (sname, type, (dim == 0 ? -1 : dim), storage);
                if (IS_STRUCT(type)) {
                    symbol_table[current_symbol_table_idx].tagidx = otag;
                }
                break;
            } else if (is_struct) {
                /* TOOD build right struct type etvc and check */
                // structure member, mtag->size is offset
                add_member(sname, type, mtag->size, storage);
                // store (correctly scaled) size of member in tag table entry
                scale_const(type, otag, &dim);
                mtag->size += dim;
            }
            else {
                // union member, offset is always zero
                add_member(sname, type, 0, storage);
                // store maximum member size in tag table entry
                scale_const(type, otag, &dim);
                if (mtag->size < dim)
                    mtag->size = dim;
            }
        }
        if (!match (T_COMMA))
            return 0;
    }
}

/**
 * initialize global objects
 * @param symbol_name
 * @param type char or integer or struct
 * @param identity
 * @param dim
 * @return 1 if variable is initialized
 */
unsigned initials(unsigned symbol_name, unsigned type, int dim, int otag) {
    int dim_unknown = 0;
    int n;

    if(dim == 0) { // allow for xx[] = {..}; declaration
        dim_unknown = 1;
    }
    /* TODO Array init */
    if (type == VOID || IS_FUNCTION(type)) {
        error("unsupported storage size");
    }
    data_segment_gdata();
    glabel(symbol_name);
    if(match(T_EQ)) {
        // an array or struct
        if(match(T_LCURLY)) {
            // aggregate initialiser
            if (IS_STRUCT(type)) {
                // aggregate is structure or pointer to structure
                dim = 0;
                struct_init(&tag_table[otag], symbol_name);
            }
            else {
                while((dim > 0) || (dim_unknown)) {
#if 0
        FIXME array of objets not simple

                    if (IS_ARRAY(type)identity == ARRAY && type == STRUCT) {
                        // array of struct
                        needbrack(T_LCURLY);
                        struct_init(&tag_table[otag], symbol_name);
                        --dim;
                        needbrack(T_RCURLY);
                    }
                    else {
#endif
                        if (init(symbol_name, type, &dim, 0)) {
                            dim_unknown++;
                        }
#if 0
                    }
#endif
                    if(match(T_COMMA) == 0) {
                        break;
                    }
                }
                if(--dim_unknown == 0)
                    /* ICK FIXME check what should occur here */
                    type++;
                else {
                    /* Pad any missing objects */
                    n = dim;
                    gen_def_storage();
                    n *= type_sizeof(type);
                    output_number(n);
                    newline();
                }
            }
            needbrack(T_RCURLY);
        // single constant
        } else {
            init(symbol_name, type, &dim, 0);
        }
    }
    code_segment_gtext();
    return type;
}

/**
 * initialise structure
 * @param tag
 */
void struct_init(TAG_SYMBOL *tag, unsigned symbol_name) {
	int dim ;
	int member_idx;
	int size = tag->size;

	member_idx = tag->member_idx;
	while (member_idx < tag->member_idx + tag->number_of_members) {
		size -= init(symbol_name, member_table[tag->member_idx + member_idx].type,
                        &dim, tag);
		++member_idx;
		/* FIXME:  not an error - zero rest */
		if ((match(T_COMMA) == 0) && (member_idx != (tag->member_idx + tag->number_of_members))) {
		    gen_def_storage();
		    output_number(size);
		    newline();
		    break;
		}
	}
}

/**
 * evaluate one initializer, add data to table
 * @param symbol_name
 * @param type
 * @param identity
 * @param dim
 * @param tag
 * @return
 *	returns size of initializer, or 0 for none (a null string is size 1)
 *
 */
int init(unsigned symbol_name, unsigned int type, int *dim, TAG_SYMBOL *tag) {
    int value, n;
    unsigned size = 1;
    /* A pointer is initialized as a word holding the address of the struct
       or string etc that directly follows */
    if (PTR(type)) {
        int x = getlabel();
        gen_def_word();
        print_label(x);
        newline();
        print_label(x);
        output_label_terminator();
        newline();
    }
    /* FIXME: may need to distinguish const string v data in future */
    if(quoted_string(&n, NULL)) {
        /* FIXME: arrays */
        if(type != CCHAR +1 && type != UCHAR + 1 && type != VOID + 1)
            error("found string: must assign to char pointer or array");
        *dim = *dim - n; /* ??? FIXME arrays of char only */
        return n;
    }
    type = type_deref(type);
    if (type == CCHAR || type == UCHAR)
        gen_def_byte();
    else if (type == CINT || type == UINT || PTR(type)) {
        size = 2;
        gen_def_word();
    } else
        error("don't know how to initialize");
    if (!number(&value))
        return 0;
    *dim = *dim - 1;
    output_number(value);
    newline();
    return size;
}

/**
 * declare local variables
 * works just like "declglb", but modifies machine stack and adds
 * symbol table entry with appropriate stack offset to find it again
 * @param typ
 * @param stclass
 * @param otag index of tag in tag_table
 */
void declare_local(unsigned typ, int stclass, int otag) {
    int     k;
    unsigned sname;

    FOREVER {
        FOREVER {
            if (endst())
                return;
            if (match(T_STAR))
                /* Eww FIXME */
                typ++;
            if (!(sname = symname()))
                illname();
            if (-1 != find_locale(sname))
                multidef (sname);
            if (match(T_LSQUARE)) {
                k = needsub();
                if (k) {
#if 0
        array type handling
                    j = ARRAY;
                    /* FIXME */
                    if (typ & CINT) {
                        k = k * INTSIZE;
                    } else if (typ == STRUCT) {
                        k = k * tag_table[otag].size;
                    }
#endif
                } else {
                    typ++;
                    k = INTSIZE;
                }
            } else {
                k = type_sizeof(typ);
            }
            if (stclass == LSTATIC) {
                add_local(sname, typ, k, LSTATIC);
                break;
            }
            if (stclass == REGISTER) {
                int r = gen_register(k, typ);
                if (r != -1) {
                    add_local(sname, typ, r, REGISTER);
                    break;
                }
            }
            /* FIXME: frame size tracking */
            if (match(T_EQ)) {
                header(H_LOCALASSIGN, sname, k);
//                gen_modify_stack(stkp);
                write_tree(expression(NO));
            } else
                header(H_LOCAL, sname, k);
//              stkp = gen_defer_modify_stack(stkp - k);
            add_local(sname, typ, stkp, AUTO);
            break;
        }
        if (!match(T_COMMA))
            return;
    }
}

/**
 * get required array size. [xx]
 * @return array size
 */
int needsub(void) {
    int num[1];

    if (match (T_RSQUARE))
        return (0);
    if (!number (num)) {
        error ("must be constant");
        num[0] = 1;
    }
    if (num[0] < 0) {
        error ("negative size illegal");
        num[0] = (-num[0]);
    }
    needbrack (T_RSQUARE);
    return (num[0]);
}

/**
 * search global table for given symbol name
 * @param sname
 * @return table index
 */
int find_global (unsigned sname) {
    int idx;

    idx = 0;
    while (idx < global_table_index) {
        if (sname == symbol_table[idx].name)
            return (idx);
        idx++;
    }
    return (-1);
}

/**
 * search local table for given symbol name
 * @param sname
 * @return table index
 */
int find_locale (unsigned sname) {
    int idx;

    idx = local_table_index;
    while (idx >= NUMBER_OF_GLOBALS) {
        idx--;
        if (sname == symbol_table[idx].name)
            return (idx);
    }
    return (-1);
}

/**
 * add new symbol to global table
 * @param sname
 * @param type
 * @param offset size in bytes
 * @param storage
 * @return new index
 */
int add_global (unsigned sname, unsigned type, int offset, int storage) {
    SYMBOL *symbol;
    if ((current_symbol_table_idx = find_global(sname)) > -1) {
        return (current_symbol_table_idx);
    }
    if (global_table_index >= NUMBER_OF_GLOBALS) {
        error ("global symbol table overflow");
        return (0);
    }
    current_symbol_table_idx = global_table_index;
    symbol = &symbol_table[current_symbol_table_idx];
    symbol->name = sname;
    symbol->type = type;
    symbol->storage = storage;
    symbol->offset = offset;
    global_table_index++;
    return (current_symbol_table_idx);
}

/**
 * add new symbol to local table
 * @param sname
 * @param identity
 * @param type
 * @param offset size in bytes
 * @param storage_class
 * @return 
 */
int add_local (unsigned sname, unsigned type, int offset, int storage_class) {
    int k;
    SYMBOL *symbol;

    if ((current_symbol_table_idx = find_locale (sname)) > -1) {
        return (current_symbol_table_idx);
    }
    if (local_table_index >= NUMBER_OF_GLOBALS + NUMBER_OF_LOCALS) {
        error ("local symbol table overflow");
        return (0);
    }
    current_symbol_table_idx = local_table_index;
    symbol = &symbol_table[current_symbol_table_idx];
    symbol->name = sname;
    symbol->type = type;
    symbol->storage = storage_class;
    if (storage_class == LSTATIC) {
        data_segment_gdata();
        print_label(k = getlabel());
        output_label_terminator();
        gen_def_storage();
        output_number(offset);
        newline();
        code_segment_gtext();
        offset = k;
    }
    symbol->offset = offset;
    local_table_index++;
    return (current_symbol_table_idx);
}

/**
 * test if next input string is legal symbol name
 */

#define IS_SYMBOL(x)	((x) & 0x8000)

unsigned symname(void) {
    if (IS_SYMBOL(token)) {
        unsigned n = token;
        next_token();
        return n;
    }
    return 0;
}

/**
 * print error message
 */
void illname(void) {
    error ("illegal symbol name");
}

/**
 * print error message
 * @param symbol_name
 * @return 
 */
void multidef(unsigned symbol_name) {
    error ("already defined");
    output_name (symbol_name);
    newline ();
}


void notvoid(int type)
{
    if (type == VOID)
        error("cannot be void type");
}
