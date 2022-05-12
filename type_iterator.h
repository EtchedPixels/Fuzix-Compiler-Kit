
extern void skip_modifiers(void);
extern unsigned type_and_name(unsigned storage, unsigned *np, unsigned needname, unsigned deftype);
extern unsigned get_type(void);
extern void type_iterator(unsigned storage, unsigned deftype, unsigned info,
    unsigned (*handler)(unsigned storage, unsigned type, unsigned name, unsigned info));

extern unsigned is_modifier(void);
extern unsigned is_type_word(void);
