/*
 * File struct.c: (12/12/12,21:31:33)
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"

/**
 * look up a tag in tag table by name
 * @param sname
 * @return index
 */
int find_tag(unsigned sname) {
    int index;
    
    index = 0;
    while (index < tag_table_index) {
        if (sname == tag_table[index].name) {
            return index;
        }
        ++index;
    }
    return -1;
}

/**
 * determine if 'sname' is a member of the struct with tag 'tag'
 * @param tag
 * @param sname
 * @return pointer to member symbol if it is, else 0
 */
SYMBOL *find_member(unsigned info, unsigned sname) {
#if 0
    /* TODO : work out final plumbing */
    SYMBOL *symbol = &tag_table[info];
    member_idx = s->member_idx;

    while (member_idx < tag->member_idx + tag->number_of_members) {
        if (member_table[member_idx].name == sname)
            return &member_table[member_idx];
        ++member_idx;
    }
#endif
    return 0;
}

/**
 * add new structure member to table
 * @param sname
 * @param identity - variable, array, pointer, function
 * @param typ
 * @param offset
 * @param storage
 * @return 
 */
void add_member(unsigned sname, unsigned type, int offset, int storage_class) {
    SYMBOL *symbol;
    if (member_table_index >= NUMMEMB) {
        error("symbol table overflow");
        return;
    }
    symbol = &member_table[member_table_index];
    symbol->name = sname;
    symbol->type = type;
    symbol->storage = storage_class;
    symbol->offset = offset;

    member_table_index++;
}

int define_struct(unsigned sname, int storage, int is_struct) {
    TAG_SYMBOL *symbol;

    //tag_table_index++;
    if (tag_table_index >= NUMTAG) {
        error("struct table overflow");
        return 0;
    }
    symbol = &tag_table[tag_table_index];
    symbol->name = sname;
    symbol->size = 0;
    symbol->member_idx = member_table_index;

    needbrack(T_LCURLY);
    do {
        do_declarations(storage, &tag_table[tag_table_index], is_struct);
    } while (!match (T_RCURLY));
    symbol->number_of_members = member_table_index - symbol->member_idx;
    return tag_table_index++;
}
