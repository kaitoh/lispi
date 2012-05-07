
#include "lispi.h"
#include "primitive.h"
#include "parser.h"
#include "objects.h"
#include "hash.h"

VALUE pri_nop(VALUE c);
VALUE pri_cons(VALUE c);
VALUE pri_car(VALUE c);
VALUE pri_cdr(VALUE c);

VALUE pri_add(VALUE c);
VALUE pri_sub(VALUE c);
VALUE pri_mal(VALUE c);
VALUE pri_div(VALUE c);
VALUE pri_list(VALUE c);

VALUE pri_display(VALUE);
VALUE pri_exit(VALUE c);

VALUE pri_is_list(VALUE c);
VALUE pri_is_eq(VALUE c);
VALUE pri_equal(VALUE c);
VALUE pri_greter(VALUE c);
VALUE pri_less(VALUE c);
VALUE pri_is_equal(VALUE c);
VALUE pri_is_null(VALUE c);
VALUE pri_is_symbol(VALUE c);
VALUE pri_is_atom(VALUE c);
VALUE pri_is_quoted(VALUE c);

VALUE spf_if(VALUE c, environment *env);
VALUE spf_cond(VALUE c, environment *env);
VALUE spf_define(VALUE c, environment *env);
VALUE spf_lambda(VALUE c, environment *env);
VALUE spf_quote(VALUE c, environment *env);
VALUE spf_begin(VALUE c, environment *env);
VALUE spf_env(VALUE c, environment *env);
VALUE spf_and(VALUE c, environment *env);
VALUE spf_or(VALUE c, environment *env);
VALUE spf_not(VALUE c, environment *env);
VALUE spf_let(VALUE c, environment *env);
VALUE spf_macro(VALUE c, environment *env);
VALUE spf_read(VALUE c, environment *env);
VALUE spf_set(VALUE c, environment *env);

VALUE (*nop)() = pri_nop;

const proc_tag pri_proc_tag_tbl[] = {
	{"cons", pri_cons},
	{"car", pri_car},
	{"cdr", pri_cdr},
	{"+", pri_add},
	{"-", pri_sub},
	{"/", pri_div},
	{"*", pri_mal},
	{"list", pri_list},
	{"list?", pri_is_list},
	{"eq?", pri_is_eq},
	{"=", pri_equal},
	{"equal?", pri_is_equal},
	{">", pri_greter},
	{"<", pri_less},
	{"null?", pri_is_null},
	{"symbol?", pri_is_symbol},
	{"atom?", pri_is_atom},
	{"quoted?", pri_is_quoted},
	{"display", pri_display},
	{"exit", pri_exit},


	{"", pri_nop}

};

// special form table
const proc_tag spf_tag_tbl[] = {
	{"if", spf_if},
	{"cond", spf_cond},
	{"define", spf_define},
	{"lambda", spf_lambda},
	{"quote", spf_quote},
	{"begin", spf_begin},
	{"and", spf_and},
	{"or", spf_or},
	{"not", spf_not},
	{"let", spf_let},
	{"macro", spf_macro},
	{"read", spf_read},
	{"set!", spf_set},
	
	{"env", spf_env},

	{"", pri_nop}
};

int is_pri_proc(VALUE proc)
{
	const proc_tag *p;
	for(p = pri_proc_tag_tbl ; p->proc != nop ; p++)
		if( (VALUE)(p->proc) == proc )
			return TRUE;
	return FALSE;
}
int is_spf(VALUE proc)
{
	const proc_tag *p;
	for(p = spf_tag_tbl ; p->proc != nop ; p++)
		if( (VALUE)(p->proc) == proc )
			return TRUE;
	return FALSE;
}


void init_pri_tbl(st_table **pri_tbl)
{
	const proc_tag *p;

	*pri_tbl = st_init_strtable();
	for(p = pri_proc_tag_tbl ; p->proc != nop ; p++)
		st_add(*pri_tbl, (long)p->proc_name, (long)p->proc);
}

void init_spf_tbl(st_table **spf_tbl)
{
	const proc_tag *p;

	*spf_tbl = st_init_strtable();
	for(p = spf_tag_tbl ; p->proc != nop ; p++)
		st_add(*spf_tbl, (long)p->proc_name, (long)p->proc);
}


/**
 * (if cond true-act )
 * (if cond true-act false-act)
 */
VALUE spf_if(VALUE c, environment *env)
{
	VALUE p = CDR(c);
	if( LTrue == eval(CAR(p), env) ) {
		p = CAR(CDR(p));
	} else {
		if( LNil !=CDR(CDR(p)) )
			p = CAR(CDR(CDR(p)));
		else // if not exist ELSE-clause, if-clause return #f
			p = LFalse;
	}
	return eval(p, env);
}

/**
 * (cond (exp1 body1)
 *       (exp2 body2)
 *       (else body ) )
 *
 * (cond ( exp1 => f1 )
 *       ( exp2 => f2 )
 *       (else felse  ) )
 *       
 * (cond ( exp1 (f1 exp1) )
 *       ( exp2 (f2 exp2) )
 *       (else felse  ) )
 */
VALUE spf_cond(VALUE c, environment *env)
{
	LCell *p, *pred;
	VALUE body;
	VALUE ret = LNil;
	VALUE exp_ret = LNil;
	int is_arrowed = FALSE;

	p = LCell( CDR(c) );
	while(LNil != (VALUE)p) { // each  "(exp body)"
		Dlist(p);
		pred = LCell( CAR(CAR(p)) );
		if(list_len(CAR(p)) == 3 && !li_strcmp("=>", SYMBOL_NAME(CAR(CDR(CAR(p)))))) {
			is_arrowed = TRUE;
			body = CAR(CDR(CDR((CAR(p)))));
		} else {
			body = CAR(CDR((CAR(p))));
		}
		Dlist(pred);
		Dlist(body);
		if( (!IS_SELF_EVALUATION(pred) && !li_strcmp("else", LSymbol(pred)->name) )
				|| (exp_ret = eval((VALUE)pred, env)) != LFalse) {
			if(is_arrowed) {
				ret = eval( cons( body, cons(exp_ret, LNil)) , env );
			} else {
				ret = eval((VALUE)body, env);
			}
			break;
		}
		D();
		p = LCell(CDR(p));
	}
	return ret;
}


/**
 * (define a b)
 * (define a () )
 * (define (a x) () ()...)
 *  -> a (lambda (x) (begin () ()))
 */
#define MAKE_LAMBDA(param, body) cons((VALUE)new_LSymbol("lambda"), cons((VALUE)param, (VALUE)body))
#define SPF_DEFINE_BODY(c) CDR(CDR(c))
VALUE spf_define(VALUE c, environment *env)
{
	VALUE var, val;
	VALUE param, body;

	if( IS_SYMBOL(CAR(CDR(c))) ) {
		var = CAR(CDR(c));
		param = LNil;
	} else {
		var = CAR(CAR(CDR(c)));
		param = CDR(CAR(CDR(c)));
	}

	if( LNil == param || IS_SELF_EVALUATION(CAR(CDR(CDR(c)))) || IS_SYMBOL(CAR(CDR(CDR(c)))) ) {
		val = CAR(CDR(CDR(c)));
	} else {
		body = CDR(CDR(c));
		val = MAKE_LAMBDA(param, body);
	}
	Dlist(val);
	st_add(env->symbol_tbl, (long)(LSymbol(var)->name), (long)eval(val, env));
	return LTrue;
}


LProc *make_proc(LCell *param, LCell *body, environment *env)
{
	LProc *p = new_LProc(env);

	Dlist(param);
	Dlist(body);
	p->car = (VALUE)param;
	p->cdr = cons( cons((VALUE)new_LSymbol("begin"), (VALUE)body) , LNil);
	Dlist(p);
	return p;
}
// ex. (lambda (x y) (+ x y) (* x y)... )
VALUE spf_lambda(VALUE c, environment *env)
{
	return (VALUE)make_proc( LCell(CAR(CDR(c))), LCell(CDR(CDR(c))), env);
}
/**
 * C is (quote hoge)
 */
VALUE spf_quote(VALUE c, environment *env)
{
	Dlist(c);
	return (VALUE)CAR(CDR(c));
}
// C is (begin (hoge) (fuga) ...)
VALUE spf_begin(VALUE c, environment *env)
{
	VALUE ret = LNil;
	LCell *p = LCell(CDR(c));
	while(LNil != (VALUE)p) {
		ret = eval(CAR(p), env);
		p = LCell(CDR(p));
	}
	return ret;
}


VALUE spf_env(VALUE c, environment *env)
{
	print_env(env);
	return c;
}

// (and ( ) ( ) )
VALUE spf_and(VALUE c, environment *env)
{
	VALUE ret;
	ret = eval(CAR(CDR(c)), env);
	if(ret == LFalse) {
		return ret;
	} else {
		ret = eval(CAR(CDR(CDR(c))), env);
		return (LFalse == ret) ? LFalse : LTrue ;
	}
}
VALUE spf_or(VALUE c, environment *env)
{
	VALUE ret;
	ret = eval(CAR(CDR(c)), env);
	if(ret == LFalse) {
		ret = eval(CAR(CDR(CDR(c))), env);
		return (LFalse == ret) ? LFalse : LTrue ;
	} else {
		return LTrue;
	}
}
VALUE spf_not(VALUE c, environment *env)
{
	VALUE ret;
	ret = eval(CAR(CDR(c)), env);
	return (LFalse == ret) ? LTrue : LFalse ;
}
// (let ((v1 e1) (v2 e2)...) <body>)
// --> ( (lambda (v1 v2...) <body>)
//       e1 e1... ) => return
#define SPF_LET_BODY(c) LCell(CDR(CDR(c)))
#define SPF_LET_PARAM(c) CAR(CAR(c))
#define SPF_LET_ARG(c) CAR(CDR(CAR(c)))
VALUE spf_let(VALUE c, environment *env)
{
	LCell *body = SPF_LET_BODY(c);
	LCell *params;
	LCell *args;

	c = CAR(CDR(c));
	params = LCell( cons(SPF_LET_PARAM(c), LNil) );
	args = LCell( cons(SPF_LET_ARG(c), LNil) );
	while( LNil != (c = CDR(c))) {
		LCell(params)->cdr = cons( SPF_LET_PARAM(c), LNil);
		LCell(args)->cdr = cons( SPF_LET_ARG(c), LNil);
	}
	return eval( cons( MAKE_LAMBDA(params, body), (VALUE)args), env);
}
VALUE spf_macro(VALUE c, environment *env)
{
	return LNil;
}

VALUE spf_read(VALUE c, environment *env)
{
	LString *s = LString(CAR(CDR(c)));
	Dlist(s);
	if(IS_STRING(s)) {
		set_pbuff(LString(s)->str);
		return eval(parse(), env);
	}
	return LNil;
}
// (set! x (+ 1 2) )
VALUE spf_set(VALUE c, environment *env)
{
	VALUE ret;
	ret = st_rewrite( env->symbol_tbl, (long)(LSymbol(CAR(CDR(c)))->name),
			(long)eval(CAR(CDR(CDR(c))), env) );
	Denv(env);
	return LTrue;
}

VALUE pri_nop(VALUE c)
{
	return LNil;
}

// (cons 1 (2))
VALUE pri_cons(VALUE c)
{
	LCell *new_c = new_LCell();
	Dlist(c);
	
	new_c->car = CAR(c);
	new_c->cdr = CAR(CDR(c));
	return (VALUE)new_c;
}
VALUE pri_car(VALUE c)
{
	return CAR(CAR(c));
}
VALUE pri_cdr(VALUE c)
{
	return CDR(CAR(c));
}
VALUE pri_add(VALUE c)
{
	int i = 0;
	VALUE p = c;

	while(LNil != p && IS_FIXNUM(CAR(p)) ) {
		i +=  FIX2INT(CAR(p));
		p = CDR(p);
	}
	
	return INT2FIX(i);
}
VALUE pri_sub(VALUE c)
{
	int i = 0;
	VALUE p = c;

	if(LNil != p && IS_FIXNUM(CAR(p)) ) {
		i =  FIX2INT(CAR(p));
		p = CDR(p);
		while(LNil != p && IS_FIXNUM(CAR(p)) ) {
			i -=  FIX2INT(CAR(p));
			p = CDR(p);
		}
	}
	
	return INT2FIX(i);
}
VALUE pri_mal(VALUE c)
{
	int i = 1;
	VALUE p = c;

	while(LNil != p && IS_FIXNUM(CAR(p)) ) {
		i *=  FIX2INT(CAR(p));
		p = CDR(p);
	}
	
	return INT2FIX(i);
}
VALUE pri_div(VALUE c)
{
	int i = 0;
	VALUE p = c;

	if(LNil != p && IS_FIXNUM(CAR(p)) ) {
		i =  FIX2INT(CAR(p));
		p = CDR(p);
		while(LNil != p && IS_FIXNUM(CAR(p)) ) {
			i /=  FIX2INT(CAR(p));
			p = CDR(p);
		}
	}
	
	return INT2FIX(i);
}
// (a b c d e...)
VALUE pri_list(VALUE c)
{
	return c;
}

void print_list(const char *pre, VALUE v)
{
	print_str(pre);
	print_str(" ");
	pri_display(v);
	println_str("");
}

VALUE pri_display(VALUE v)
{
	VALUE p = v;

	if(IS_UNDEF(p)) {
		return LFalse;
	}

	if( LNil == p) {
		print_str("nil ");
	} else if (LTrue == p) {
		print_str("#t ");
	} else if (LFalse == p) {
		print_str("#f ");
	} else if(IS_FIXNUM(p)) {
		print_int(FIX2INT(p));
		print_str(" ");
	} else if( !IS_IMMEDIATE(p) ) {
		if( IS_STRING(p) ) {
			print_str("\"");
			print_str(LString(p)->str);
			print_str("\" ");
		} else if( IS_SYMBOL(p) ) {
			print_str(SYMBOL_NAME(p));
			print_str(" ");
		} else if (IS_CONSCELL(p)) {
			print_str("(");
			pri_display( CAR(p) );
			print_str(" . ");
			pri_display( CDR(p) );
			print_str(")");
		} else if (IS_LIST(p)) {
			print_str("(");
			while( LNil != p) {
				pri_display(LCell(p)->car);
				p = LCell(p)->cdr;
			}
			print_str(") ");
		}
	}

	return LTrue;
}

VALUE pri_exit(VALUE c)
{
	li_exit(0);
	return 0;
}

VALUE pri_is_list(VALUE c)
{
	VALUE v = CAR(c);
	return (LNil != v && IS_LIST(v) ) ?  LTrue : LFalse;
}
VALUE pri_is_eq(VALUE c)
{
	return (CAR(c) == CAR(CDR(c))) ? LTrue : LFalse;
}
VALUE pri_equal(VALUE c)
{
	return (CAR(c) == CAR(CDR(c))) ? LTrue : LFalse;
}
VALUE pri_is_equal(VALUE c)
{
	return (CAR(c) == CAR(CDR(c))) ? LTrue : LFalse;
}
VALUE pri_greter(VALUE c)
{
	return (FIX2INT(CAR(c)) > FIX2INT(CAR(CDR(c)))) ? LTrue : LFalse;
}
VALUE pri_less(VALUE c)
{
	return (FIX2INT(CAR(c)) < FIX2INT(CAR(CDR(c)))) ? LTrue : LFalse;
}
VALUE pri_is_null(VALUE c)
{
	Dlist(c);
	return (LNil == CAR(c)) ? LTrue : LFalse;
}
VALUE pri_is_symbol(VALUE c)
{
	VALUE v = CAR(c);
	return ( IS_SYMBOL(v) ) ? LTrue : LFalse;
}
VALUE pri_is_atom(VALUE c)
{
	VALUE v = CAR(c);
	return (IS_SELF_EVALUATION(v)) ? LTrue : LFalse;
}
VALUE pri_is_quoted(VALUE c)
{
	VALUE v = CAR(c);
	return (IS_SELF_EVALUATION(v)) ? LTrue : LFalse;
}
