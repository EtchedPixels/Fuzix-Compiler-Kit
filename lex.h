extern unsigned token;
extern unsigned line_num;
/* value will change as we handle more type stuff */
extern unsigned token_value;

extern void next_token(void);
extern unsigned match(unsigned);
extern void require(unsigned);
extern void need_semicolon(void);
extern void junk(void);
extern unsigned symname(void);

extern unsigned quoted_string(int *len);
