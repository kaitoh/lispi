
#ifndef EVAL_H
#define EVAL_H

#include "lispi.h"
#include "objects.h"

extern VALUE cons(VALUE, VALUE);
extern environment *init_top_env(void);
extern VALUE eval(VALUE s, environment *e);
extern void print_env(environment *env);

#endif
