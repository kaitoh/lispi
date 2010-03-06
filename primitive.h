
#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "objects.h"
#include "hash.h"
#include "eval.h"

typedef VALUE (*pri_proc)(VALUE );
typedef VALUE (*spf_proc)(VALUE , environment *);

typedef struct _proc_tag {
	const char *proc_name;
	VALUE (*proc)();
} proc_tag;


extern VALUE (*nop)();
extern LCell *concat(LCell *, VALUE);
extern void print_list(const char *, VALUE);
extern void init_spf_tbl(st_table **);
extern void init_pri_tbl(st_table **);
int is_pri_proc(VALUE proc);
int is_spf(VALUE proc);

#endif
