/*
    monitor.c   	-	simple 6809 monitor, disassembler, hex, s19 and binary support
    Copyright (C) 2001  Arto Salmi
                        Joze Fabcic

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


#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <readline/readline.h>

#include "6809.h"

enum addr_mode
{
  _illegal, _implied,  _imm_byte, _imm_word, _direct,  _extended,
  _indexed, _rel_byte, _rel_word, _reg_post, _sys_post,_usr_post
};

enum opcode
{
  _undoc,_abx,  _adca, _adcb, _adda, _addb, _addd, _anda, _andb,
  _andcc,_asla, _aslb, _asl,  _asra, _asrb, _asr,  _bcc,  _lbcc,
  _bcs,  _lbcs, _beq,  _lbeq, _bge,  _lbge, _bgt,  _lbgt, _bhi,
  _lbhi, _bita, _bitb, _ble,  _lble, _bls,  _lbls, _blt,  _lblt,
  _bmi,  _lbmi, _bne,  _lbne, _bpl,  _lbpl, _bra,  _lbra, _brn,
  _lbrn, _bsr,  _lbsr, _bvc,  _lbvc, _bvs,  _lbvs, _clra, _clrb,
  _clr,  _cmpa, _cmpb, _cmpd, _cmps, _cmpu, _cmpx, _cmpy, _coma,
  _comb, _com,  _cwai, _daa,  _deca, _decb, _dec,  _eora, _eorb,
  _exg,  _inca, _incb, _inc,  _jmp,  _jsr,  _lda,  _ldb,  _ldd,
  _lds,  _ldu,  _ldx,  _ldy,  _leas, _leau, _leax, _leay, _lsra,
  _lsrb, _lsr,  _mul,  _nega, _negb, _neg,  _nop,  _ora,  _orb,
  _orcc, _pshs, _pshu, _puls, _pulu, _rola, _rolb, _rol,  _rora,
  _rorb, _ror,  _rti,  _rts,  _sbca, _sbcb, _sex,  _sta,  _stb,
  _std,  _sts,  _stu,  _stx,  _sty,  _suba, _subb, _subd, _swi,
  _swi2, _swi3, _sync, _tfr,  _tsta, _tstb, _tst,  _reset
};

char *mne[] =
{
  "???",  "ABX",  "ADCA", "ADCB", "ADDA", "ADDB", "ADDD", "ANDA", "ANDB",
  "ANDCC","ASLA", "ASLB", "ASL",  "ASRA", "ASRB", "ASR",  "BCC",  "LBCC",
  "BCS",  "LBCS", "BEQ",  "LBEQ", "BGE",  "LBGE", "BGT",  "LBGT", "BHI",
  "LBHI", "BITA", "BITB", "BLE",  "LBLE", "BLS",  "LBLS", "BLT",  "LBLT",
  "BMI",  "LBMI", "BNE",  "LBNE", "BPL",  "LBPL", "BRA",  "LBRA", "BRN",
  "LBRN", "BSR",  "LBSR", "BVC",  "LBVC", "BVS",  "LBVS", "CLRA", "CLRB",
  "CLR",  "CMPA", "CMPB", "CMPD", "CMPS", "CMPU", "CMPX", "CMPY", "COMA",
  "COMB", "COM",  "CWAI", "DAA",  "DECA", "DECB", "DEC",  "EORA", "EORB",
  "EXG",  "INCA", "INCB", "INC",  "JMP",  "JSR",  "LDA",  "LDB",  "LDD",
  "LDS",  "LDU",  "LDX",  "LDY",  "LEAS", "LEAU", "LEAX", "LEAY", "LSRA",
  "LSRB", "LSR",  "MUL",  "NEGA", "NEGB", "NEG",  "NOP",  "ORA",  "ORB",
  "ORCC", "PSHS", "PSHU", "PULS", "PULU", "ROLA", "ROLB", "ROL",  "RORA",
  "RORB", "ROR",  "RTI",  "RTS",  "SBCA", "SBCB", "SEX",  "STA",  "STB",
  "STD",  "STS",  "STU",  "STX",  "STY",  "SUBA", "SUBB", "SUBD", "SWI",
  "SWI2", "SWI3", "SYNC", "TFR",  "TSTA", "TSTB", "TST",  "RESET"
};

typedef struct
{ 
  UINT8 code;
  UINT8 mode;
} opcode_t;

opcode_t codes[256] =
{
  {_neg    ,_direct    },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_com    ,_direct    },
  {_lsr    ,_direct    },
  {_undoc  ,_illegal   },	
  {_ror    ,_direct    },
  {_asr    ,_direct    },
  {_asl    ,_direct    },
  {_rol    ,_direct    },
  {_dec    ,_direct    },
  {_undoc  ,_illegal   },
  {_inc    ,_direct    },
  {_tst    ,_direct    },
  {_jmp    ,_direct    },
  {_clr    ,_direct    },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_nop    ,_implied   },
  {_sync   ,_implied   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_lbra   ,_rel_word  },
  {_lbsr   ,_rel_word  },
  {_undoc  ,_illegal   },
  {_daa    ,_implied   },
  {_orcc   ,_imm_byte  },
  {_undoc  ,_illegal   },
  {_andcc  ,_imm_byte  },
  {_sex    ,_implied   },
  {_exg    ,_reg_post  },
  {_tfr    ,_reg_post  },
  {_bra    ,_rel_byte  },
  {_brn    ,_rel_byte  },
  {_bhi    ,_rel_byte  },
  {_bls    ,_rel_byte  },
  {_bcc    ,_rel_byte  },
  {_bcs    ,_rel_byte  },
  {_bne    ,_rel_byte  },
  {_beq    ,_rel_byte  },
  {_bvc    ,_rel_byte  },
  {_bvs    ,_rel_byte  },
  {_bpl    ,_rel_byte  },
  {_bmi    ,_rel_byte  },
  {_bge    ,_rel_byte  },
  {_blt    ,_rel_byte  },
  {_bgt    ,_rel_byte  },
  {_ble    ,_rel_byte  },
  {_leax   ,_indexed   },
  {_leay   ,_indexed   },
  {_leas   ,_indexed   },
  {_leau   ,_indexed   },
  {_pshs   ,_sys_post  },
  {_puls   ,_sys_post  },
  {_pshu   ,_usr_post  },
  {_pulu   ,_usr_post  },
  {_undoc  ,_illegal   },
  {_rts    ,_implied   },
  {_abx    ,_implied   },
  {_rti    ,_implied   },
  {_cwai   ,_imm_byte  },
  {_mul    ,_implied   },
  {_reset  ,_implied   },
  {_swi    ,_implied   },
  {_nega   ,_implied   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_coma   ,_implied   },
  {_lsra   ,_implied   },
  {_undoc  ,_illegal   },
  {_rora   ,_implied   },
  {_asra   ,_implied   },
  {_asla   ,_implied   },
  {_rola   ,_implied   },
  {_deca   ,_implied   },
  {_undoc  ,_illegal   },
  {_inca   ,_implied   },
  {_tsta   ,_implied   },
  {_undoc  ,_illegal   },
  {_clra   ,_implied   },
  {_negb   ,_implied   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_comb   ,_implied   },
  {_lsrb   ,_implied   },
  {_undoc  ,_illegal   },
  {_rorb   ,_implied   },
  {_asrb   ,_implied   },
  {_aslb   ,_implied   },
  {_rolb   ,_implied   },
  {_decb   ,_implied   },
  {_undoc  ,_illegal   },
  {_incb   ,_implied   },
  {_tstb   ,_implied   },
  {_undoc  ,_illegal   },
  {_clrb   ,_implied   },
  {_neg    ,_indexed   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_com    ,_indexed   },
  {_lsr    ,_indexed   },
  {_undoc  ,_illegal   },
  {_ror    ,_indexed   },
  {_asr    ,_indexed   },
  {_asl    ,_indexed   },
  {_rol    ,_indexed   },
  {_dec    ,_indexed   },
  {_undoc  ,_illegal   },
  {_inc    ,_indexed   },
  {_tst    ,_indexed   },
  {_jmp    ,_indexed   },
  {_clr    ,_indexed   },
  {_neg    ,_extended  },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_com    ,_extended  },
  {_lsr    ,_extended  },
  {_undoc  ,_illegal   },
  {_ror    ,_extended  },
  {_asr    ,_extended  },
  {_asl    ,_extended  },
  {_rol    ,_extended  },
  {_dec    ,_extended  },
  {_undoc  ,_illegal   },
  {_inc    ,_extended  },
  {_tst    ,_extended  },
  {_jmp    ,_extended  },
  {_clr    ,_extended  },
  {_suba   ,_imm_byte  },
  {_cmpa   ,_imm_byte  },
  {_sbca   ,_imm_byte  },
  {_subd   ,_imm_word  },
  {_anda   ,_imm_byte  },
  {_bita   ,_imm_byte  },
  {_lda    ,_imm_byte  },
  {_undoc  ,_illegal   },
  {_eora   ,_imm_byte  },
  {_adca   ,_imm_byte  },
  {_ora    ,_imm_byte  },
  {_adda   ,_imm_byte  },
  {_cmpx   ,_imm_word  },
  {_bsr    ,_rel_byte  },
  {_ldx    ,_imm_word  },
  {_undoc  ,_illegal   },
  {_suba   ,_direct    },
  {_cmpa   ,_direct    },
  {_sbca   ,_direct    },
  {_subd   ,_direct    },
  {_anda   ,_direct    },
  {_bita   ,_direct    },
  {_lda    ,_direct    },
  {_sta    ,_direct    },
  {_eora   ,_direct    },
  {_adca   ,_direct    },
  {_ora    ,_direct    },
  {_adda   ,_direct    },
  {_cmpx   ,_direct    },
  {_jsr    ,_direct    },
  {_ldx    ,_direct    },
  {_stx    ,_direct    },
  {_suba   ,_indexed   },
  {_cmpa   ,_indexed   },
  {_sbca   ,_indexed   },
  {_subd   ,_indexed   },
  {_anda   ,_indexed   },
  {_bita   ,_indexed   },
  {_lda    ,_indexed   },
  {_sta    ,_indexed   },
  {_eora   ,_indexed   },
  {_adca   ,_indexed   },
  {_ora    ,_indexed   },
  {_adda   ,_indexed   },
  {_cmpx   ,_indexed   },
  {_jsr    ,_indexed   },
  {_ldx    ,_indexed   },
  {_stx    ,_indexed   },
  {_suba   ,_extended  },
  {_cmpa   ,_extended  },
  {_sbca   ,_extended  },
  {_subd   ,_extended  },
  {_anda   ,_extended  },
  {_bita   ,_extended  },
  {_lda    ,_extended  },
  {_sta    ,_extended  },
  {_eora   ,_extended  },
  {_adca   ,_extended  },
  {_ora    ,_extended  },
  {_adda   ,_extended  },
  {_cmpx   ,_extended  },
  {_jsr    ,_extended  },
  {_ldx    ,_extended  },
  {_stx    ,_extended  },
  {_subb   ,_imm_byte  },
  {_cmpb   ,_imm_byte  },
  {_sbcb   ,_imm_byte  },
  {_addd   ,_imm_word  },
  {_andb   ,_imm_byte  },
  {_bitb   ,_imm_byte  },
  {_ldb    ,_imm_byte  },
  {_undoc  ,_illegal   },
  {_eorb   ,_imm_byte  },
  {_adcb   ,_imm_byte  },
  {_orb    ,_imm_byte  },
  {_addb   ,_imm_byte  },
  {_ldd    ,_imm_word  },
  {_undoc  ,_illegal   },
  {_ldu    ,_imm_word  },
  {_undoc  ,_illegal   },
  {_subb   ,_direct    },
  {_cmpb   ,_direct    },
  {_sbcb   ,_direct    },
  {_addd   ,_direct    },
  {_andb   ,_direct    },
  {_bitb   ,_direct    },
  {_ldb    ,_direct    },
  {_stb    ,_direct    },
  {_eorb   ,_direct    },
  {_adcb   ,_direct    },
  {_orb    ,_direct    },
  {_addb   ,_direct    },
  {_ldd    ,_direct    },
  {_std    ,_direct    },
  {_ldu    ,_direct    },
  {_stu    ,_direct    },
  {_subb   ,_indexed   },
  {_cmpb   ,_indexed   },
  {_sbcb   ,_indexed   },
  {_addd   ,_indexed   },
  {_andb   ,_indexed   },
  {_bitb   ,_indexed   },
  {_ldb    ,_indexed   },
  {_stb    ,_indexed   },
  {_eorb   ,_indexed   },
  {_adcb   ,_indexed   },
  {_orb    ,_indexed   },
  {_addb   ,_indexed   },
  {_ldd    ,_indexed   },
  {_std    ,_indexed   },
  {_ldu    ,_indexed   },
  {_stu    ,_indexed   },
  {_subb   ,_extended  },
  {_cmpb   ,_extended  },
  {_sbcb   ,_extended  },
  {_addd   ,_extended  },
  {_andb   ,_extended  },
  {_bitb   ,_extended  },
  {_ldb    ,_extended  },
  {_stb    ,_extended  },
  {_eorb   ,_extended  },
  {_adcb   ,_extended  },
  {_orb    ,_extended  },
  {_addb   ,_extended  },
  {_ldd    ,_extended  },
  {_std    ,_extended  },
  {_ldu    ,_extended  },
  {_stu    ,_extended  }
};

opcode_t codes10[256] =
{
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_lbrn   ,_rel_word  },
  {_lbhi   ,_rel_word  },
  {_lbls   ,_rel_word  },	
  {_lbcc   ,_rel_word  },
  {_lbcs   ,_rel_word  },
  {_lbne   ,_rel_word  },
  {_lbeq   ,_rel_word  },	
  {_lbvc   ,_rel_word  },
  {_lbvs   ,_rel_word  },
  {_lbpl   ,_rel_word  },
  {_lbmi   ,_rel_word  },	
  {_lbge   ,_rel_word  },
  {_lblt   ,_rel_word  },
  {_lbgt   ,_rel_word  },
  {_lble   ,_rel_word  },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_swi2   ,_implied   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_cmpd   ,_imm_word  },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_cmpy   ,_imm_word  },
  {_undoc  ,_illegal   },
  {_ldy    ,_imm_word  },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_cmpd   ,_direct    },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_cmpy   ,_direct    },
  {_undoc  ,_illegal   },
  {_ldy    ,_direct    },
  {_sty    ,_direct    },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_cmpd   ,_indexed   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_cmpy   ,_indexed   },
  {_undoc  ,_illegal   },
  {_ldy    ,_indexed   },
  {_sty    ,_indexed   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_cmpd   ,_extended  },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_cmpy   ,_extended  },
  {_undoc  ,_illegal   },
  {_ldy    ,_extended  },
  {_sty    ,_extended  },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_lds    ,_imm_word  },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_lds    ,_direct    },
  {_sts    ,_direct    },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_lds    ,_indexed   },
  {_sts    ,_indexed   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_lds    ,_extended  },
  {_sts    ,_extended  }
};

opcode_t codes11[256] =
{
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_swi3   ,_implied   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_cmpu   ,_imm_word  },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_cmps   ,_imm_word  },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_cmpu   ,_direct    },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_cmps   ,_direct    },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_cmpu   ,_indexed   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_cmps   ,_indexed   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_cmpu   ,_extended  },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_cmps   ,_extended  },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },	
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   },
  {_undoc  ,_illegal   }
};

char *reg[] =
{
 "D", "X", "Y", "U", "S", "PC","??","??",
 "A", "B", "CC","DP","??","??","??","??" 
};

char index_reg[] = { 'X','Y','U','S' };

char *off4[] =
{
  "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",
  "8",  "9", "10", "11", "12", "13", "14", "15",
"-16","-15","-14","-13","-12","-11","-10", "-9",
 "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1"
};

#define RDBYTE (memory[0xffff & pc++])
#define RDWORD (fetch1=(memory[0xffff & pc++] << 8),fetch1|memory[0xffff & pc++])

int dasm (char *buf, int opc)
{ /* returns the number of bytes that compose the current instruction */
  UINT8 op, am;
  char *op_str;
  int pc = opc & 0xffff;
  char R;
  int fetch1;  /* the first (MSB) fetched byte, used in macro RDWORD */

  op = RDBYTE;

  if (op == 0x10)
  {
    op = RDBYTE;
    am = codes10[op].mode;
    op = codes10[op].code;  
  }
  else if (op == 0x11)
  {
    op = RDBYTE;
    am = codes11[op].mode;
    op = codes11[op].code;  
  }
  else
  {
    am = codes[op].mode;
    op = codes[op].code;  
  }

  op_str = (char *)mne[op];

  switch (am)
  {
    case _illegal:  sprintf (buf, "???"); break;
    case _implied:  sprintf (buf, "%s ",op_str);     break;
    case _imm_byte: sprintf (buf, "%s #$%02X",op_str, RDBYTE); break;
    case _imm_word: sprintf (buf, "%s #$%04X",op_str, RDWORD); break;
    case _direct:   sprintf (buf, "%s <$%02X",op_str, RDBYTE); break;
    case _extended: sprintf (buf, "%s $%04X",op_str,  RDWORD); break; 

    case _indexed:  op = RDBYTE;
                    R  = index_reg[(op>>5)&0x3];

                    if((op & 0x80) == 0)
                    {
                      sprintf (buf,"%s %s,%c",op_str,off4[op&0x1f],R);
                      break;
                    }

                    switch (op & 0x1f)
                    {
                      case 0x00: sprintf (buf, "%s ,%c+",op_str,R);break;
                      case 0x01: sprintf (buf, "%s ,%c++",op_str,R);break;
                      case 0x02: sprintf (buf, "%s ,-%c",op_str,R);break;
                      case 0x03: sprintf (buf, "%s ,--%c",op_str,R);break;
                      case 0x04: sprintf (buf, "%s ,%c",op_str,R);break;
                      case 0x05: sprintf (buf, "%s B,%c",op_str,R);break;
                      case 0x06: sprintf (buf, "%s A,%c",op_str,R);break;
                      case 0x08: sprintf (buf, "%s $%02X,%c",op_str,RDBYTE,R);break;
                      case 0x09: sprintf (buf, "%s $%04X,%c",op_str,RDWORD,R); break;
                      case 0x0B: sprintf (buf, "%s D,%c",op_str,R);break;
                      case 0x0C: sprintf (buf, "%s $%02X,PC",op_str,RDBYTE);break;
                      case 0x0D: sprintf (buf, "%s $%04X,PC",op_str,RDWORD); break;
                      case 0x11: sprintf (buf, "%s [,%c++]",op_str,R);break;
                      case 0x13: sprintf (buf, "%s [,--%c]",op_str,R);break;
                      case 0x14: sprintf (buf, "%s [,%c]",op_str,R);break;
                      case 0x15: sprintf (buf, "%s [B,%c]",op_str,R);break;
                      case 0x16: sprintf (buf, "%s [A,%c]",op_str,R);break;
                      case 0x18: sprintf (buf, "%s [$%02X,%c]",op_str,RDBYTE,R);break;
                      case 0x19: sprintf (buf, "%s [$%04X,%c]",op_str,RDWORD,R);break;
                      case 0x1B: sprintf (buf, "%s [D,%c]",op_str,R);break;
                      case 0x1C: sprintf (buf, "%s [$%02X,PC]",op_str,RDBYTE);break;
                      case 0x1D: sprintf (buf, "%s [$%04X,PC]",op_str,RDWORD);break;
                      case 0x1F: sprintf (buf, "%s [$%04X]",op_str,RDWORD);break;
                      default:   sprintf (buf, "%s ??",op_str); break;
                    }
                    break;

    case _rel_byte: fetch1 = ((INT8)RDBYTE);  
                    sprintf (buf, "%s $%04X",op_str, (fetch1 + pc) & 0xffff);
                    break;

    case _rel_word: sprintf (buf, "%s $%04X",op_str, (RDWORD + pc) & 0xffff);
                    break;

    case _reg_post: op = RDBYTE;
                    sprintf (buf, "%s %s,%s",op_str, reg[op>>4],reg[op&15]);
                    break;

    case _usr_post:
    case _sys_post: op = RDBYTE;
                    sprintf (buf, "%s ",op_str);

                    if(op&0x80) strcat (buf, "PC,");
                    if(op&0x40) strcat (buf, am == _usr_post ? "S," : "U," );
                    if(op&0x20) strcat (buf, "Y,");
                    if(op&0x10) strcat (buf, "X,");
                    if(op&0x08) strcat (buf, "DP,");
                    if((op&0x06) == 0x06) strcat (buf, "D,");
                    else
                    {
                      if(op&0x04) strcat (buf, "B,");
                      if(op&0x02) strcat (buf, "A,");
                    }
                    if(op&0x01) strcat (buf, "CC,");
                    buf[strlen(buf)-1]='\0';
                    break;

  }
  return pc - opc;
}


#define MAX_BREAKS		15
#define CLEAR_BREAK		0xffffffff

int brkpoints[MAX_BREAKS];

int inst_count = 0;
int do_break = 0;
int monitor_on = 0;

void str_tolower (char *str)
{
  if (*str == '\0' || str == NULL) return;
  do {*str = tolower(*str); str++; } while (*str != '\0');
}

int str_getnumber (char *str)
{
  int val = 0;
  
  if (*str == '\0' || str == NULL) return val;

  switch (*str)
  {
    case '$': sscanf(str+1,"%x",(unsigned int *)&val); break;
    case '@': sscanf(str+1,"%o",(unsigned int *)&val); break;
    default : sscanf(str,"%d",&val);   break;
    case '%': for (;;)
              {
                str++;
                if (*str < '0' || *str > '1') break;
                val = (val << 1) | (*str & 1);
              }
              break;
  }
  return val;
}

int str_scan (char *str, char *table[],int maxi)
{
  int i = 0;

  for (;;)
  {
    while(isgraph(*str) == 0 && *str != '\0') str++;
    if (*str == '\0') return i;
    table[i] = str;
    if (maxi-- == 0) return i;

    while(isgraph(*str) != 0) str++;
    if (*str == '\0') return i;
    *str++ = '\0';
    i++;
  }
}

typedef struct
{
  int   cmd_nro;
  char *cmd_string;
} command_t;

enum cmd_numbers
{
  CMD_HELP,
  CMD_DUMP,
  CMD_DASM,
  CMD_RUN,
  CMD_GO,
  CMD_SET,
  CMD_CLR,
  CMD_SHOW,
 
  CMD_SETBRK,
  CMD_CLRBRK,
  CMD_BRKSHOW,
  CMD_BRKOFF,
  CMD_BRKON,

  CMD_QUIT,
  CMD_EXIT,

  REG_PC,
  REG_X,
  REG_Y,
  REG_S,
  REG_U,
  REG_D,
  REG_CC,
  REG_DP,
  REG_A,
  REG_B,

  CCR_E,
  CCR_F,
  CCR_H,
  CCR_I,
  CCR_N,
  CCR_Z,
  CCR_V,
  CCR_C
};

command_t cmd_table [] =
{
  { CMD_HELP,    "help"    },
  { CMD_HELP,    "h"       },
  { CMD_HELP,    "?"       },
  { CMD_DUMP,    "dump"    },
  { CMD_DUMP,    "d"       },
  { CMD_DASM,    "dis"     },
  { CMD_RUN,     "step"    },
  { CMD_RUN,     "s"       },
  { CMD_GO,      "go"      },
  { CMD_GO,      "g"       },
  { CMD_SET,     "set"     },
  { CMD_CLR,     "clr"     },
  { CMD_SHOW,    "show"    },
  { CMD_SHOW,    "sh"      },
  { CMD_SETBRK,  "brk"     },
  { CMD_CLRBRK,  "cbrk"    },
  { CMD_BRKSHOW, "bshow"   },
  { CMD_BRKOFF,  "boff"    },
  { CMD_BRKON,   "bon"     },
  { CMD_QUIT,    "quit"    },
  { CMD_QUIT,    "q"       },
  { CMD_EXIT,    "exit"    },
  { CMD_EXIT,    "x"       },

  {-1,   NULL}
};

command_t arg_table [] =
{
  { REG_PC, "PC" },
  { REG_X,  "X"  },
  { REG_Y,  "Y"  },
  { REG_S,  "S"  },
  { REG_U,  "U"  },
  { REG_D,  "D"  },
  { REG_CC, "CC" },
  { REG_DP, "DP" },
  { REG_A,  "A"  },
  { REG_B,  "B"  },
  { CCR_E,  "E"  },
  { CCR_F,  "F"  },
  { CCR_H,  "H"  },
  { CCR_I,  "I"  },
  { CCR_N,  "N"  },
  { CCR_Z,  "Z"  },
  { CCR_V,  "V"  },
  { CCR_C,  "C"  },

  { -1,    NULL  }
};

int get_command (char *str, command_t *table)
{
  int index;
  int nro = -1;

  if (*str == '\0' || str == NULL) return -1;

  str_tolower(str);

  for (index = 0; table[index].cmd_string != NULL; index++)
  {
    if (strcmp(str,table[index].cmd_string) == 0)
    {
      nro = table[index].cmd_nro;
      break;
    }
  }

  return nro;
}

static void monitor_signal (int sigtype)
{
  (void)sigtype;
  monitor_on = 1;
}

void monitor_init (int start_in_monitor)
{
  int tmp;

  for (tmp = 0; tmp < MAX_BREAKS; tmp++) brkpoints[tmp] = CLEAR_BREAK;
  inst_count = do_break = 0;
  monitor_on = start_in_monitor;
  signal(SIGINT, monitor_signal);
}

int check_break (unsigned break_pc)
{
  int temp_pc = break_pc & 0xffff;

  if (do_break != 0)
  {
    int tmp;

    for (tmp = 0; tmp < MAX_BREAKS; tmp++)
    {
      if (temp_pc == brkpoints[tmp]) return 1;
    }
  }

  if (inst_count > 0) if (--inst_count == 0) return 1;

  return 0;
}

void add_breakpoint (int break_pc)
{
  int tmp;
  int clear = -1;

  for (tmp = 0; tmp < MAX_BREAKS; tmp++)
  {
    if (brkpoints[tmp] == break_pc   ) { clear = -1; break; }
    if (brkpoints[tmp] == CLEAR_BREAK)   clear = tmp; 
  }

  if (clear == -1) printf("failed to add breakpoint\n");
  else brkpoints[clear] = break_pc;
}

void clear_breakpoint (int break_pc)
{
  int tmp;

  for (tmp = 0; tmp < MAX_BREAKS; tmp++) 
  {
    if (brkpoints[tmp] == break_pc) brkpoints[tmp] = CLEAR_BREAK;
  }
}

void show_breakpoints (void)
{
 int tmp;

  for (tmp = 0; tmp < MAX_BREAKS; tmp++) 
  {
    if (brkpoints[tmp] != CLEAR_BREAK) printf("%04X \n",brkpoints[tmp]);
  }
}

void cmd_dump (int start, int end)
{
  int addr = start;
  int lsize;
  char abuf[17];

  if (start > end)
  {
    printf("invalid arguments\n");
    return;
  }

  printf("memory dump $%04x to $%04x\n",start,end);

  for (;;)
  {
    printf("%04x: ",addr);

    for(lsize = 0;(addr < (end + 1)) && (lsize < 16);addr++,lsize++)
    {
      int mb = memory[addr];
      printf("%02x ",mb);
      abuf[lsize] = isprint(mb)?mb:'.';
    }
    abuf[lsize] = '\0';
    while (lsize++ < 16) printf("   "); 
 
    puts(abuf);

    if(addr > end) break;
  }

}

void cmd_dasm (int start, int end)
{ 
  char buf[50];
  int addr = start;
  int size;

  if (start >= end)
  {
    printf("invalid arguments\n");
    return;
  }

  do
  {
    size = dasm(buf,addr);
    printf("%04x: %-15s ;",addr,buf);

    for(;size;size--) printf("%02x ",memory[0xffff & addr++]);
    printf("\n");
  } while (addr < (end+1));
}

void cmd_show (void)
{
  int cc = get_cc();
  int pc = get_pc();
  char inst[50];
  int offset, moffset;

  moffset = dasm(inst,pc);

  printf("S  $%04X U  $%04X X  $%04X Y  $%04X   EFHINZVC\n",get_s(),get_u(),get_x(),get_y());
  printf("A  $%02X   B  $%02X   DP $%02X   CC $%02X     ",get_a(),get_b(),get_dp(),cc);
  printf("%c%c%c%c"    ,(cc&E_FLAG?'1':'.'),(cc&F_FLAG?'1':'.'),(cc&H_FLAG?'1':'.'),(cc&I_FLAG?'1':'.'));
  printf("%c%c%c%c\n",(cc&N_FLAG?'1':'.'),(cc&Z_FLAG?'1':'.'),(cc&V_FLAG?'1':'.'),(cc&C_FLAG?'1':'.'));
  printf("PC $%04X ",pc);
  for (offset=0; offset<moffset; offset++)
    printf ("%02X", memory[0xffff & (offset+pc)]);
  printf("  NextInst: %s\n",inst);
}

int monitor6809 (void)
{
  char  *cmd_str;
  char  *arg[10];
  int   arg_count;

  signal(SIGINT, monitor_signal);
  monitor_on = 0;

  cmd_show();

  for (;;)
  {
    fflush(stdout);
    fflush(stdin);
   
    cmd_str = readline("monitor>");

    if (*cmd_str == '\0') { inst_count = 1; return 0; }
    arg_count = str_scan(cmd_str,arg,5) + 1;

    switch (get_command(arg[0],cmd_table))
    {
      case CMD_HELP:    puts("\nemu6809 monitor commands:                      ");
                        puts("(h)elp                - shows this text          ");
                        puts("(d)ump start end      - dump memory start to end ");
                        puts("(d)ump int            - dump memory at int       ");
                        puts("dis    start end      - disassemble start to end "); 
                        puts("(s)tep [n]            - run n instructions, default 1");
                        puts("(g)o   addr           - start executing at addr  ");
                        puts("set    register value - put value to register    ");
                        puts("set    flag           - set CC flag              ");
                        puts("clr    register       - clear register           ");
                        puts("clr    flag           - clear CC flag            ");
                        puts("(sh)ow register       - show register hex value  ");
                        puts("(sh)ow flag           - show CC flag             ");
                        puts("brk    addr           - set breakpoint to addr   ");
                        puts("cbrk   addr           - clear breakpoint at addr ");
                        puts("bshow                 - show breakpoints         ");
                        puts("boff                  - disable all breakpoints  ");
                        puts("bon                   - enable all breakpoints   ");
                        puts("(q)uit                - quit simulator           ");
                        puts("e(x)it                - exit monitor (like go)   ");
                        puts("number formats $hex, @oct, %bin, dec           \n");
                        continue;

      case CMD_DUMP:    if (arg_count == 2)
                          arg[2] = arg[1];
			if (arg_count < 2 || arg_count > 3)
			  break;
                        cmd_dump(str_getnumber(arg[1])&0xffff,str_getnumber(arg[2])&0xffff);
                        continue;

      case CMD_DASM:    if (arg_count != 3) break;
                        cmd_dasm(str_getnumber(arg[1])&0xffff,str_getnumber(arg[2])&0xffff);
                        continue;

      case CMD_RUN:     if (arg_count > 2) break;
			inst_count= 1;
                        if (arg_count == 2)
                          inst_count = str_getnumber(arg[1]) & 0xffff;
                        if (inst_count != 0) return 0;
                        break;

      case CMD_GO:      if (arg_count != 2) break;
                        set_pc(str_getnumber(arg[1]) & 0xffff);
                        return 0;

      case CMD_SET:     if (arg_count == 3)
                        {
                          int temp = str_getnumber(arg[2]);

                          switch (get_command(arg[1],arg_table))
                          {
                            case REG_PC: set_pc(temp); continue;
                            case REG_X:  set_x(temp);  continue;
                            case REG_Y:  set_y(temp);  continue;
                            case REG_S:  set_s(temp);  continue;
                            case REG_U:  set_u(temp);  continue;
                            case REG_D:  set_d(temp);  continue;
                            case REG_CC: set_cc(temp); continue;
                            case REG_DP: set_dp(temp); continue;
                            case REG_A:  set_a(temp);  continue;
                            case REG_B:  set_b(temp);  continue;
                          }
                          break; /* invalid argument */
                        }
                        else if (arg_count == 2)
                        {
                          switch (get_command(arg[1], arg_table))
                          {
                            case CCR_E:  set_cc(get_cc()|E_FLAG); continue;
                            case CCR_F:  set_cc(get_cc()|F_FLAG); continue;
                            case CCR_H:  set_cc(get_cc()|H_FLAG); continue;
                            case CCR_I:  set_cc(get_cc()|I_FLAG); continue;
                            case CCR_N:  set_cc(get_cc()|N_FLAG); continue;
                            case CCR_Z:  set_cc(get_cc()|Z_FLAG); continue;
                            case CCR_V:  set_cc(get_cc()|V_FLAG); continue;
                            case CCR_C:  set_cc(get_cc()|C_FLAG); continue;
                          }
                          break; /* invalid argument */
                        }
                        break; /* invalid number of arguments */
                     
      case CMD_CLR:     if (arg_count != 2) break;

                        switch (get_command(arg[1], arg_table))
                        {
                          case REG_PC: set_pc(0); continue;
                          case REG_X:  set_x(0);  continue;
                          case REG_Y:  set_y(0);  continue;
                          case REG_S:  set_s(0);  continue;
                          case REG_U:  set_u(0);  continue;
                          case REG_D:  set_d(0);  continue;
                          case REG_DP: set_dp(0); continue;
                          case REG_CC: set_cc(0); continue;
                          case REG_A:  set_a(0);  continue;
                          case REG_B:  set_b(0);  continue;

                          case CCR_E:  set_cc(get_cc()&~E_FLAG); continue;
                          case CCR_F:  set_cc(get_cc()&~F_FLAG); continue;
                          case CCR_H:  set_cc(get_cc()&~H_FLAG); continue;
                          case CCR_I:  set_cc(get_cc()&~I_FLAG); continue;
                          case CCR_N:  set_cc(get_cc()&~N_FLAG); continue;
                          case CCR_Z:  set_cc(get_cc()&~Z_FLAG); continue;
                          case CCR_V:  set_cc(get_cc()&~V_FLAG); continue; 
                          case CCR_C:  set_cc(get_cc()&~C_FLAG); continue; 
                        }
                        break; /* invalid argument */
 
      case CMD_SHOW:    if (arg_count != 2) {cmd_show(); continue; }
  
                        switch (get_command(arg[1], arg_table))
                        {
                          case REG_PC: printf("PC: $%04X\n",get_pc()); continue;
                          case REG_X:  printf("X:  $%04X\n",get_x());  continue;
                          case REG_Y:  printf("Y:  $%04X\n",get_y());  continue;
                          case REG_S:  printf("S:  $%04X\n",get_s());  continue;
                          case REG_U:  printf("U:  $%04X\n",get_u());  continue;
                          case REG_D:  printf("D:  $%04X\n",get_d());  continue;
                          case REG_DP: printf("DP: $%02X\n",get_dp()); continue;
                          case REG_CC: printf("CC: $%02X\n",get_cc()); continue;
                          case REG_A:  printf("A:  $%02X\n",get_a());  continue;
                          case REG_B:  printf("B:  $%02X\n",get_b());  continue;

                          case CCR_E:  printf("CC:E %c\n",get_cc()&E_FLAG?'1':'0'); continue;
                          case CCR_F:  printf("CC:F %c\n",get_cc()&F_FLAG?'1':'0'); continue;
                          case CCR_H:  printf("CC:H %c\n",get_cc()&H_FLAG?'1':'0'); continue;
                          case CCR_I:  printf("CC:I %c\n",get_cc()&I_FLAG?'1':'0'); continue;
                          case CCR_N:  printf("CC:N %c\n",get_cc()&N_FLAG?'1':'0'); continue;

                          case CCR_Z:  printf("CC:Z %c\n",get_cc()&Z_FLAG?'1':'0'); continue;
                          case CCR_V:  printf("CC:V %c\n",get_cc()&V_FLAG?'1':'0'); continue; 
                          case CCR_C:  printf("CC:C %c\n",get_cc()&C_FLAG?'1':'0'); continue; 
                        }
                        break; /* invalid argument */

      case CMD_SETBRK:  if (arg_count != 2) break;
                        add_breakpoint(str_getnumber(arg[1]) & 0xffff);
                        continue;

      case CMD_CLRBRK:  if (arg_count != 2) break;
                        clear_breakpoint(str_getnumber(arg[1]) & 0xffff);
                        continue;

      case CMD_BRKSHOW: show_breakpoints (); continue;

      case CMD_BRKOFF:  puts("break points OFF"); do_break = 0; continue;
      case CMD_BRKON:   puts("break points ON");  do_break = 1; continue;
      case CMD_QUIT:    cpu_quit = 0; return 1;
      case CMD_EXIT:    return 0;
      default:
      case -1:          puts("invalid command"); continue;
    }

    puts("invalid argument or number of arguments");
  }
}
