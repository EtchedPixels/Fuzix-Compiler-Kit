#include <inttypes.h>

#define AH		0
#define AL		1
#define BH		2
#define BL		3
#define XH		4
#define XL		5
#define YH		6
#define YL		7
#define ZH		8
#define ZL		9
#define SH		10
#define SL		11
#define CH		12
#define CL		13
#define PH		14
#define PL		15

#define A		0
#define B		2
#define X		4
#define Y		6
#define Z		8
#define S		10	/* Just a convention it seems */
#define C		12	/* Flags ? */
#define P		14	/* PC */

#define ONE_SECOND_NS 1000000000.0

extern uint8_t mem_read8(uint16_t addr);
extern uint8_t mem_read8_debug(uint16_t addr);
extern void mem_write8(uint16_t addr, uint8_t val);
extern void halt_system(void);
extern uint16_t ee200_pc(void);
extern void set_pc_debug(uint16_t new_pc);
extern void reg_write_debug(uint8_t r, uint8_t v);
extern void regpair_write_debug(uint8_t r, uint16_t v);
extern unsigned ee200_execute_one(unsigned trace);
extern void ee200_set_switches(unsigned switches);
extern unsigned ee200_halted(void);
extern void ee200_init(void);
extern void cpu_assert_irq(unsigned ipl);
extern void cpu_deassert_irq(unsigned ipl);

void disassemble(unsigned op);
