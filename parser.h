
#ifndef L_LEX_H
#define L_LEX_H


#include "objects.h"

#define ISWORD(c) ( isprefix(c) || li_isdigit(c) || isspecial_suf(c) )


// ---------------- define for Lex --------------------
enum  tok {
	tOPENP = 1, // (
	tCLOSEP, // )
	tNUM,
	tSYM,
	tQUOTE, // '
	tID,
	tSTRING,
	tFUNC,
	tW_QUOTE, // "
	tCOMMENT, // ;
	tBACK_QUOTE, // `
	tCOMMA, // ,
	tPERIOD, // .
	tVEC_BEGIN, // #(
	tSPACE,
	tCOLON,

	NUM_TOK
};


typedef struct _token {
	enum tok type;
	VALUE val;
} token;

typedef struct _prog_buff {
	char *buff;
	int size;
	int i;
} prog_buff;


extern VALUE parse(void);
extern char *get_cl(void);
extern void init_pbuff(void);
extern void set_pbuff(char *);
extern void init_cl();

#endif
