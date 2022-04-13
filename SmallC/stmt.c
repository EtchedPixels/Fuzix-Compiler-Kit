/*
 * File stmt.c: 2.1 (83/03/20,16:02:17)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

/**
 * statement parser
 * called whenever syntax requires a statement.  this routine
 * performs that statement and returns a number telling which one
 * @param func func is true if we require a "function_statement", which
 * must be compound, and must contain "statement_list" (even if
 * "declaration_list" is omitted)
 * @return statement type
 */
int statement(int func) {
    if (token == T_EOF)
        return 0;
    lastst = 0;
    if (func) {
        if (match (T_LCURLY)) {
            do_compound (YES);
            return (lastst);
        } else
            error ("function requires compound statement");
    }
    if (match (T_LCURLY))
        do_compound (NO);
    else {
        do_statement ();
    }
    return (lastst);
}

/**
 * declaration
 */
int statement_declare(void) {
    if (match(T_REGISTER))
        do_local_declares(REGISTER);
    else if (match(T_AUTO))
        do_local_declares(DEFAUTO);
    else if (match(T_STATIC))
        do_local_declares(LSTATIC);
    else if (do_local_declares(AUTO)) ;
    else
        return (NO);
    return (YES);
}

/**
 * local declarations
 * @param stclass
 * @return 
 */
int do_local_declares(int stclass) {
    int type = 0;
    int otag;   // tag of struct object being declared
    int sflag;  // TRUE for struct definition, zero for union
    unsigned sname;

    if ((sflag=match(T_STRUCT)) || match(T_UNION)) {
        if ((sname = symname()) == 0) { // legal name ?
            illname();
        }
        if ((otag=find_tag(sname)) == -1) { // structure not previously defined
            otag = define_struct(sname, stclass, sflag);
        }
        declare_local(STRUCT, stclass, otag);
    } else if ((type = get_type()) != -1) {
        declare_local(type, stclass, -1);
    } else if (stclass == LSTATIC || stclass == DEFAUTO) {
        declare_local(CINT, stclass, -1);
    } else {
        return(0);
    }
    need_semicolon();
    return(1);
}

/**
 * non-declaration statement
 */
void do_statement(void) {
    if (match (T_IF)) {
        doif ();
        lastst = STIF;
    } else if (match (T_WHILE)) {
        dowhile ();
        lastst = STWHILE;
    } else if (match (T_SWITCH)) {
        doswitch ();
        lastst = STSWITCH;
    } else if (match (T_DO)) {
        dodo ();
        need_semicolon ();
        lastst = STDO;
    } else if (match (T_FOR)) {
        dofor ();
        lastst = STFOR;
    } else if (match (T_RETURN)) {
        doreturn ();
        need_semicolon ();
        lastst = STRETURN;
    } else if (match (T_BREAK)) {
        dobreak ();
        need_semicolon ();
        lastst = STBREAK;
    } else if (match (T_CONTINUE)) {
        docont();
        need_semicolon ();
        lastst = STCONT;
    } else if (match (T_SEMICOLON))
        ;
    else if (match (T_CASE)) {
        docase ();
        lastst = statement (NO);
    } else if (match (T_DEFAULT)) {
        dodefault ();
        lastst = statement (NO);
#if 0
    } else if (match ("#asm")) {
        doasm ();
        lastst = STASM;
#endif
    } else if (match (T_LCURLY))
        do_compound (NO);
    else {
        write_tree(expression (YES));
/*      if (match (":")) {
            dolabel ();
            lastst = statement (NO);
        } else {
*/          need_semicolon ();
            lastst = STEXP;
/*      }
*/  }
}

/**
 * compound statement
 * allow any number of statements to fall between "{" and "}"
 * 'func' is true if we are in a "function_statement", which
 * must contain "statement_list"
 */
void do_compound(int func) {
        int     decls;

        decls = YES;
        ncmp++;
        while (!match (T_RCURLY)) {
                if (input_eof)
                        return;
                if (decls) {
                        if (!statement_declare ()) {
                                /* Any deferred movement now happens */
//                                gen_modify_stack(stkp);
                                decls = NO;
                        }
                } else
                        do_statement ();
        }
        ncmp--;
}

/**
 * "if" statement
 */
void doif(void) {
        int     fstkp, flab1, flab2;
        int     flev;

        flev = local_table_index;
        fstkp = stkp;
        flab1 = getlabel ();
        /* FIXME: sort label id stuff out later */
        header(H_IF, flab1, 0);
        test (flab1, FALSE);
        statement (NO);
        local_table_index = flev;
        if (!match (T_ELSE)) {
                footer(H_IF, flab1, 0);
                return;
        }
        header(H_ELSE, flab1, 0);
        statement (NO);
        footer(H_IF, flab1, 0);
        local_table_index = flev;
}

/**
 * "while" statement
 */
void dowhile(void) {
        WHILE ws;

        ws.symbol_idx = local_table_index;
        ws.stack_pointer = stkp;
        ws.type = WSWHILE;
        ws.case_test = getlabel ();
        ws.while_exit = getlabel ();
        addwhile (&ws);
        header(H_WHILE, ws.case_test, 0);
        test (ws.while_exit, FALSE);
        statement (NO);
        local_table_index = ws.symbol_idx;
        footer(H_WHILE, ws.case_test, 0);
        delwhile ();
}

/**
 * "do" statement
 */
void dodo(void) {
        WHILE ws;

        ws.symbol_idx = local_table_index;
        ws.stack_pointer = stkp;
        ws.type = WSDO;
        ws.body_tab = getlabel ();
        ws.case_test = getlabel ();
        ws.while_exit = getlabel ();
        addwhile (&ws);
        header(H_DO, ws.body_tab, 0);
        statement (NO);
        if (!match (T_WHILE)) {
                error ("missing while");
                return;
        }
        header(H_DOWHILE, ws.body_tab, 0);
        test (ws.body_tab, TRUE);
        footer(H_DO, ws.body_tab, 0);
        local_table_index = ws.symbol_idx;
        delwhile ();
}

/**
 * "for" statement
 */
void dofor(void) {
        WHILE ws;
        WHILE *pws;

        ws.symbol_idx = local_table_index;
        ws.stack_pointer = stkp;
        ws.type = WSFOR;
        ws.case_test = getlabel ();
        ws.incr_def = getlabel ();
        ws.body_tab = getlabel ();
        ws.while_exit = getlabel ();
        addwhile (&ws);
        pws = readwhile ();
        header(H_FOR, ws.case_test, 0);
        needbrack (T_LPAREN);
        if (!match (T_SEMICOLON)) {
                write_tree(expression (YES));
                need_semicolon ();
        } else
                write_null_tree();
        if (!match (T_SEMICOLON)) {
                write_tree(expression (YES));
                need_semicolon ();
        } else {
                write_null_tree();
                pws->case_test = pws->body_tab;
        }
        if (!match (T_RPAREN)) {
                write_tree(expression (YES));
                needbrack (T_RPAREN);
        } else {
                write_null_tree();
                pws->incr_def = pws->case_test;
        }
        statement (NO);
        footer(H_FOR, ws.case_test, 0);
        local_table_index = pws->symbol_idx;
        delwhile ();
}

/**
 * "switch" statement
 */
void doswitch(void) {
        WHILE ws;
        WHILE *ptr;

        ws.symbol_idx = local_table_index;
        ws.stack_pointer = stkp;
        ws.type = WSSWITCH;
        ws.case_test = swstp;
        ws.body_tab = getlabel ();
        ws.incr_def = ws.while_exit = getlabel ();
        addwhile (&ws);
        header(H_SWITCH, ws.case_test, 0);
        needbrack (T_LPAREN);
        write_tree(expression (YES));
        needbrack (T_RPAREN);
        statement (NO);
        ptr = readswitch ();
        dumpsw (ptr);
        local_table_index = ptr->symbol_idx;
        swstp = ptr->case_test;
        footer(H_SWITCH, ws.case_test, 0);
        delwhile ();
}

/**
 * "case" label
 */
void docase(void) {
        int     val;

        val = 0;
        if (readswitch ()) {
                /* TODO: expression as const */
                if (!number (&val))
                        error ("bad case label");
                addcase (val);
                if (!match (T_COLON))
                        error ("missing colon");
                /* Types etc .. probably an expr ?? */
                header(H_CASE, val, 0);
        } else
                error ("no active switch");
}

/**
 * "default" label
 */
void dodefault(void) {
        WHILE *ptr;
        int        lab;

        if ((ptr = readswitch ()) != 0) {
                if (!match (T_COLON))
                        error ("missing colon");
                header(H_DEFAULT, 0, 0);
        } else
                error ("no active switch");
}

/**
 * "return" statement
 */
void doreturn(void) {
        header(H_RETURN, 0, 0);
        if (endst () == 0)
                write_tree(expression (YES));
        else
                write_null_tree();
}

/**
 * "break" statement
 */
void dobreak(void) {
        WHILE *ptr;

        if ((ptr = readwhile ()) == 0)
                return;
        /* FIXME: how to pass relevant info to match to ptr/WHILE */
        header(H_BREAK, 0, 0);
}

/**
 * "continue" statement
 */
void docont(void) {
        WHILE *ptr; //int     *ptr;

        if ((ptr = findwhile ()) == 0)
                return;
        /* Sort out label idents ?? */
        header(H_CONTINUE, 0, 0);
#if 0
        if (ptr->type == WSFOR)
                gen_jump (ptr->incr_def);
        else
                gen_jump (ptr->case_test);
#endif
}

/**
 * dump switch table
 */
void dumpsw(WHILE *ws) {
        int     i,j;

        data_segment_gdata ();
        generate_label (ws->body_tab);
        if (ws->case_test != swstp) {
                j = ws->case_test;
                while (j < swstp) {
                        gen_def_word ();
                        i = 4;
                        while (i--) {
                                output_number (swstcase[j]);
                                output_byte (',');
                                print_label (swstlab[j++]);
                                if ((i == 0) | (j >= swstp)) {
                                        newline ();
                                        break;
                                }
                                output_byte (',');
                        }
                }
        }
        gen_def_word ();
        print_label (ws->incr_def);
        output_string (",0");
        newline ();
        code_segment_gtext ();
}
