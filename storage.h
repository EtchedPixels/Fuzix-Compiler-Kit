extern unsigned get_storage(unsigned defstorage);
extern unsigned is_storage_word(void);

#define AUTO		1
#define LSTATIC		2
#define STATIC		3
#define EXTDEF		4
#define EXTERN		5
#define BSS		6	/* Only used to pass info to the code generator */
extern void put_typed_data(struct node *n, unsigned storage);
extern void put_padding_data(unsigned space, unsigned storage);


