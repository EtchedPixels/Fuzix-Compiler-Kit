/*
    6809.h   	-	header for monitor and simulator
    Copyright (C) 2001  Arto Salmi

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef M6809_H
#define M6809_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char UINT8;
typedef signed char INT8;

typedef unsigned short UINT16;
typedef signed short INT16;

typedef unsigned int UINT32;
typedef signed int INT32;

#define E_FLAG 0x80
#define F_FLAG 0x40
#define H_FLAG 0x20
#define I_FLAG 0x10
#define N_FLAG 0x08
#define Z_FLAG 0x04
#define V_FLAG 0x02
#define C_FLAG 0x01

extern UINT8 *memory;

/* 6809.c */
extern int cpu_quit;
extern int cpu_execute (int);
extern void cpu_reset (void);

extern unsigned get_a  (void);
extern unsigned get_b  (void);
extern unsigned get_cc (void);
extern unsigned get_dp (void);
extern unsigned get_x  (void);
extern unsigned get_y  (void);
extern unsigned get_s  (void);
extern unsigned get_u  (void);
extern unsigned get_pc (void);
extern unsigned get_d  (void);
extern void set_a  (unsigned);
extern void set_b  (unsigned);
extern void set_cc (unsigned);
extern void set_dp (unsigned);
extern void set_x  (unsigned);
extern void set_y  (unsigned);
extern void set_s  (unsigned);
extern void set_u  (unsigned);
extern void set_pc (unsigned);
extern void set_d  (unsigned);

/* monitor.c */
extern int monitor_on;
extern int check_break (unsigned);
extern void monitor_init (void); 
extern int monitor6809 (void);
extern int dasm (char *, int);

extern int load_hex (char *);
extern int load_s19 (char *);
extern int load_bin (char *,int);

#endif /* M6809_H */
