
#include "lispi.h"
#include "parser.h"
#include "hash.h"
#include "primitive.h"
#include "eval.h"


environment *new_env(environment *super);
VALUE eval(VALUE s, environment *env);
VALUE apply(VALUE proc, VALUE args);

static st_table *pri_tbl; // primitive proc table
static st_table *spf_tbl; // special form table

environment top_env; // Top of the environment

/**
 * extend environment.
 */
environment *new_env(environment *super)
{
	environment *e = OBJ_Malloc(environment);
	e->symbol_tbl = st_init_strtable();
	e->super = super;
	return e;
}

VALUE lookup_variable(VALUE s, environment *env)
{
	VALUE val = LUndef;
	environment *e = env;

	while(NULL != e && !st_lookup(e->symbol_tbl, (long)(LSymbol(s)->name), (long *)&val)) {
		e = e->super;
	}
	return val;
}
int is_spf_tag(VALUE proc, VALUE *v)
{
	if(st_lookup(spf_tbl, (long)(LSymbol(proc)->name), (long *)v)) return TRUE;
	else return FALSE;

}

VALUE list( VALUE a, VALUE b)
{
	VALUE c = cons(cons(a,b), LNil);
	return c;

}
VALUE cons( VALUE a, VALUE b)
{
	LCell *c = new_LCell();
	c->car = a;
	c->cdr = b;
	return (VALUE)c;
}
VALUE operator(VALUE s)
{
	return CAR(s);
}
VALUE operands(VALUE s)
{
	return CDR(s);
}
VALUE list_of_value(VALUE s, environment *env)
{
	if( LNil == s )
		return LNil;
	else {
		LCell *c;
		c = (LCell*)cons( eval(CAR(s), env) , list_of_value(CDR(s), env) );
		Dlist(c);
		return (VALUE)c;
	}
}

VALUE eval(VALUE s, environment *env)
{
	VALUE val = LUndef;
	Dlist(s);

	if( IS_SELF_EVALUATION(s) ) {
		return s;
	} else if(IS_VARIABLE(s)) {
		val = lookup_variable(s, env);
		if(IS_UNDEF(val)) {
            Denv(env);
			print_str("#<undef>: "), print_str(LSymbol(s)->name), println_str("");
		}
		return val;
	} else if(IS_LIST(s)) {
		D();
		if(is_spf_tag(operator(s), &val)) {
			D();
			return ((spf_proc)val)(s , env);
		} else {
			return apply( eval(operator(s), env), list_of_value(operands(s), env) );
		}
	} else {
		Ds("Unexpected");
		li_exit(-1);
	}
	return LUndef;
}

VALUE apply_pri_proc(VALUE proc, VALUE args)
{
	D();
	return ((pri_proc)proc)(args);
}
environment *extend_env(VALUE params, VALUE args, environment *base)
{
	environment *new = new_env(base);
	while(LNil != params) {
		st_add( new->symbol_tbl, (long)(LSymbol(CAR(params))->name), (long)CAR(args) );
		params = CDR(params);
		args = CDR(args);
	}
	Denv(new);
	return new;
}
VALUE apply(VALUE proc, VALUE args)
{
	Dlist(proc);
	Dlist(args);
	if( is_pri_proc(proc) ) {
		VALUE v;
		D();
		v = apply_pri_proc(proc, args);
		Dlist(v);
		return v;
	} else if( IS_UNDEF(proc) ) {
		return LUndef;
	} else if( !IS_ATOM(proc) && IS_PROC(proc) ){
		VALUE ret;
		environment *e;

		D();
		e = extend_env(CAR(proc), args, LProc(proc)->env); 
		ret = eval(CAR(CDR(proc)), e);
		FREE_ENV(e);
		return ret;
	} else {
		Dexit(-1);
	}
	return LUndef;
}


environment *init_top_env(void)
{
	init_spf_tbl(&spf_tbl);
	init_pri_tbl(&pri_tbl);

	top_env.symbol_tbl = pri_tbl;
	top_env.super = NULL;
	return new_env(&top_env);
}

void print_env(environment *env)
{
	environment *e = env;

	println_str("---");
	while(NULL != e) {
		print_hash(e->symbol_tbl);
		println_str("---");
		e = e->super;
	}
}
