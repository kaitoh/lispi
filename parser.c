
#include "lispi.h"
#include "parser.h"
#include "hash.h"
#include "objects.h"
#include "primitive.h"


static int isspecial_pre(char c);
static int isspecial_suf(char c);
static int isprefix(char c);
static char *peek_p();
static VALUE read_next(token *t);

static prog_buff p_buff;


char *strndup(const char *str, int len)
{
	char *s = (char *)Malloc(len+1);
	int i = 0;
	for(i = 0 ; i < len ; i++) {
		s[i] = str[i];
	}
    s[i+1] = '\0';
	return s;
}

int isspecial_pre(char c)
{
	switch(c) {
		case '!':
		case '$':
		case '%':
		case '&':
		case '*':
		case '/':
		case ':':
		case '<':
		case '=':
		case '>':
		case '?':
		case '^':
		case '_':
		case '~':

		case '#':
			return 1;
		default:
			return 0;
	}
}
int isspecial_suf(char c)
{

	switch(c) {
		case '+':
		case '-':
		case '.':
		case '@':
			return 1;
		default:
			return 0;
	}
}

int isprefix(char c)
{
	return (isspecial_pre(c) || li_isalpha(c));
}


/**
 * if C is'nt spacer return 0. else spaceer type ID.
 */
enum tok isspacer_t(char c)
{
	switch(c) {
		case '\t':
		case '\r':
		case '\n':
		case ' ': 
			return tSPACE;
		case '(': 
			return tOPENP;
		case ')': 
			return tCLOSEP;
		case '"': 
			return tW_QUOTE;
		case ';': 
			return tCOMMENT;
		default:
			return FALSE;
	}
}

/////// ------------- lex ---------------


unsigned int is_eol()
{
	unsigned char c = p_buff.buff[p_buff.i];
	return ('\0' == c || NULL == c || ';' == c);
}
char *peek_p()
{
	return &(p_buff.buff[p_buff.i]);
}
unsigned char peek()
{
	return p_buff.buff[p_buff.i];
}
unsigned char get()
{
	unsigned char c = p_buff.buff[p_buff.i];
	p_buff.i++;
	return c;
}
char nextc()
{
	return p_buff.buff[p_buff.i+1];
}
char back()
{
	char c;
	if(p_buff.i <= 0) return '\0';

	c = p_buff.buff[p_buff.i-1];
	p_buff.i--;
	return c;
}
char skip_space()
{
	char c = p_buff.buff[p_buff.i];
	while(c != '\0' && li_isspace(c)) {
		p_buff.i++;
		c = p_buff.buff[p_buff.i];
	}
	return c;
}

char *get_string()
{
	char *s; 
	char *e; 
	char *ret;

	s = peek_p();
	e = li_strchr(s, '"');
	if(NULL != e) {
		ret = strndup(s, e-s);
		p_buff.i += (e-s)+1;
	} else {
		ret = strndup(s,0);
	}
	return ret;
}

/**
 * To get next token and length LEN.
 */
token *scanner()
{
	const char *s; 
	unsigned char c;
	int n = 0;
	enum tok ttype = NUM_TOK;
	token *t = OBJ_Malloc(token);

	t->type = NUM_TOK;

	skip_space();
	if(is_eol()) return NULL;
	skip_space();

	s = peek_p();
	c = get();
	if( (ttype = isspacer_t(c)) ) { // ( ) ; "
		t->type = ttype;
		n++;
	} else if ( ',' == c ) {
		t->type = tCOLON;
		n++;
	} else if ( '\'' == c ) {
		t->type = tQUOTE;
		n++;
	} else if (ISWORD(c)) {
		int num = 0;
		int pogneg = 1;
		unsigned char st = 0;
		if(li_isdigit(c)) {
			st = 1;
			num = c - '0';
			t->type = tNUM;
		} else if('-' == c) {
			pogneg = -1;
			num = 0;
			st = 2;
			t->type = tID;
		} else {
			st = 3;
			t->type = tID;
		}

		n++;
		c = get();
		while(ISWORD(c)) {
			n++;
			switch(st) {
				case 1: // num
					if(li_isdigit(c)) {
						num = num*10 + c - '0';
					} else {
						t->type = tID; 
						st = 3; 
					}
					break;
				case 2:
					if(li_isdigit(c)) {
						num = c - '0';
						t->type = tNUM;
						st = 1;
					} else {
						t->type = tID;
						st = 3;
					}
					break;
				case 3:
					t->type = tID;
					break;
				default: 
					D();
					li_exit(1);
					break;
			}
			c = get();
		}
		back();
		if(t->type == tNUM) {
			t->val = (VALUE)(pogneg*num);
		} else {
			t->val = (VALUE)strndup(s, n);
		}
		return t;
	} else {
		print_str("--");
		print_int(c);
		print_str("--");
		D();
		li_exit(-1);
	}
	t->val = (VALUE)strndup(s, n);
	return t;
}

/////// ------------- parse ---------------
VALUE make_list()
{
	token *t;
	LCell *p;

	t = scanner();
	if( NULL == t || t->type == tCLOSEP) {
		return (VALUE)LNil;
	} else {
		p = new_LCell();
		p->car = (VALUE)read_next(t);
		p->cdr = make_list();
		return (VALUE)p;
	}
}

VALUE make_quote()
{
	LCell *p = new_LCell();
	LCell *top;
	
	top = p;
	p->car = (VALUE)new_LSymbol("quote");
	p->cdr = (VALUE)new_LCell();
	p = LCell( CDR(p) );

	p->car = parse();

	return (VALUE)top;
}

VALUE make_string()
{
	LString *s;
	s = new_LString(get_string());
	return (VALUE)s;
}
	

VALUE read_next(token *t)
{
	VALUE p;

	if( NULL == t ) return LNil;

	switch(t->type) {
		case tOPENP:
			p = make_list();
			break;
		case tCLOSEP:
			p = LNil;
			break;
		case tW_QUOTE:
			p = make_string();
			break;
		case tQUOTE:
			p = make_quote();
			break;
		case tNUM:
			p = INT2FIX(t->val);
			break;
		case tID:
			if(!li_strcmp("#f", (char *)t->val)) p = LFalse;
			else if(!li_strcmp("#t", (char *)t->val)) p = LTrue;
			else if(!li_strcmp("nil", (char *)t->val)) p = LNil;
			else p = (VALUE)new_LSymbol((const char *)(t->val));
			break;
		default:
			break;
	}
	return p;
}

VALUE parse()
{
	VALUE p;
	token *t;

	t = scanner();
	p = read_next(t);
	return p;
}

void set_pbuff(char *str)
{
	p_buff.buff = str;
	p_buff.size = li_strlen(str);
	p_buff.i = 0;
}

/**
 * @return 0: read success, -1: EOF
 */
#define BUFF_SIZE 1024

void init_cl()
{
	p_buff.buff = (char *)Malloc(sizeof(char)*BUFF_SIZE);
}

char *get_cl()
{
	const char *prompt = "li> ";

    print_str(prompt);
	get_line(p_buff.buff, BUFF_SIZE);
	p_buff.size = li_strlen(p_buff.buff);
	p_buff.i = 0;

	return p_buff.buff;
}

void init_pbuff()
{
	p_buff.size = 0;
	p_buff.buff = NULL;
	p_buff.i = 0;
}
