
#include "lispi.h"
#include "objects.h"

unsigned int list_len(VALUE v)
{
	unsigned int cnt = 0;
	while(!IS_IMMEDIATE(v) && IS_LIST(v)) {
		cnt++;
		v = CDR(v);
	}
	return cnt;
}

LSymbol *new_LSymbol(const char *str)
{
	LSymbol *c = OBJ_Malloc(LSymbol);
	if(NULL == c) Dexit(1); 

	LBasic(c)->klass = kSYMBOL;
	c->name = (char *)str;
	return c;
}

LString *new_LString(const char *str)
{
	LString *c = OBJ_Malloc(LString);
	if(NULL == c) Dexit(1); 

	LBasic(c)->klass = kSTRING;
	c->str = (char *)str;
	return c;
}

LCell *new_LCell(void)
{
	LCell *c = OBJ_Malloc(LCell);
	if(NULL == c) Dexit(1); 

	LBasic(c)->klass = kCELL;
	c->car = LNil;
	c->cdr = LNil;
	return c;
}

LProc *new_LProc(environment *env)
{
	LProc *c = OBJ_Malloc(LProc);
	if(NULL == c) Dexit(1); 
	LBasic(c)->klass = kPROC;
	c->car = LNil;
	c->cdr = LNil;
	c->env = env;
	return c;
}

