extern uint8_t mem_read8(uint16_t addr);
extern uint8_t mem_read8_debug(uint16_t addr);
extern void mem_write8(uint16_t addr, uint8_t val);

extern void tms7k_reset(void);
extern int tms7k_execute(int clocks);
extern void tms7k_trace(FILE *f);
