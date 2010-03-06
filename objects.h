
#ifndef OBJECTS_H
#define OBJECTS_H

#include "hash.h"

// ---------------- define for some type of Object --------------------
/**
 * VALUE is pointer to  EXP, CELL, ...
 *  or #t #f nil FIXNUM SYMBOL_ID
 */
typedef unsigned long VALUE;
typedef unsigned long ID;


#define LFalse 0 // => #f
#define LTrue 2 // => #t, not #f
#define LNil 4 // => nil, '()
#define LUndef 6 // all symbol's default value

#define FIXNUM_FLAG 0x01
#define INT2FIX(i) ((VALUE)(((long)(i) << 1) | FIXNUM_FLAG))
#define FIX2INT(i) ((long)(((long)(i)) >> 1))

#define SYMBOL_FLAG 0x0e
#define ID2SYM(i) ((VALUE)(((long)(i)<<8) | SYMBOL_FLAG))
#define SYM2ID(i) ((i)>>8)
#define SYMBOL_NAME(s) LSymbol(s)->name

// IS_*** 
#define IS_UNDEF(v) ( LUndef == (VALUE)v )
#define IS_ATOM(v) (LTrue == (VALUE)v || LFalse == (VALUE)v || LNil == (VALUE)v ) 
#define IS_BOOL(v) (LTrue == (VALUE)v || LFalse == (VALUE)v ) 
#define IS_FIXNUM(i) ( ((VALUE)(i)) & 0x01 )

#define IS_IMMEDIATE(v) ( IS_UNDEF(v) || IS_ATOM(v) || IS_FIXNUM(v) ) // V is not struct type

//#define IS_SYMBOL(i) ( ((VALUE)(i)) & 0x0e )
#define IS_SYMBOL(v) ( kSYMBOL == LBasic(v)->klass )
#define IS_STRING(v) ( kSTRING == LBasic(v)->klass )
#define IS_QUOTE(v) ( kQUOTE == LBasic(v)->klass )
#define IS_PROC(v) ( kPROC == LBasic(v)->klass )
#define IS_CELL(v) ( kCELL == LBasic(v)->klass )
#define IS_LIST_KLASS(v) ( IS_CELL(v) || IS_PROC(v) )

#define IS_SELF_EVALUATION(v) ( IS_FIXNUM(v) || IS_ATOM(v) || IS_STRING(v) ) 
#define IS_VARIABLE(v) ( IS_SYMBOL(v) )

#define IS_LIST(v) ( !IS_IMMEDIATE(v) && IS_LIST_KLASS(v) \
		&& (LNil == CDR(v) || IS_LIST_KLASS(CDR(v))) )

#define IS_CONSCELL(v) ( !IS_IMMEDIATE(v) && IS_LIST_KLASS(v) && LNil != CDR(v) \
		&& (IS_BOOL(CDR(v)) || IS_FIXNUM(CDR(v)) || IS_SYMBOL(CDR(v)) ) )


// Access
#define GET_KLASS(p) (LBasic(p)->klass)

#define LBasic(p) ((LBasic*)p)
#define LSymbol(p) ((LSymbol*)p)
#define LString(p) ((LString*)p)
#define LCell(p) ((LCell*)p)
#define LProc(p) ((LProc*)p)
#define CAR(p) (LCell(p)->car)
#define CDR(p) (LCell(p)->cdr)


// Objects
enum klass {
	kSTRING,
	kSEXP,
	kCELL,
	kSYMBOL,
	kVARIABLE,
	kPROC,

	num_klass
};

/**
 * environment in evaluation.
 */
typedef struct _environment {
	st_table *symbol_tbl; // hash with strint => VALUE
	struct _environment *super; // pointer to basically environment.
} environment;

typedef struct _lbasic {
	VALUE klass;
} LBasic;

// LSymbol.klass = {kSTRING, kQUOTE, kVARIABLE}
typedef struct _lsymbol {
	LBasic basic;
	char *name;
} LSymbol;

typedef struct _lstring {
	LBasic basic;
	char *str;
} LString;

typedef struct _lcell {
	LBasic basic;
	VALUE car;
	VALUE cdr;
} LCell;

typedef struct _lproc {
	LBasic basic;
	VALUE car;
	VALUE cdr;
	environment *env;
} LProc;


extern unsigned int list_len(VALUE v);
extern LCell *new_LCell();
extern LSymbol *new_LSymbol(const char *);
extern LString *new_LString(const char *);
extern LProc *new_LProc(environment *);

#endif
