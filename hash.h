
#ifndef HASH_H
#define HASH_H


// ---------------- define for Hash Table --------------------

#define HASH_MAX_DENCITY 5
#define NUM_INCREMENT_REHASH 5


// hash table
typedef struct _st_table {
	struct _st_hash_type *type;
	int num_entries;
	int num_bins;
	struct _st_table_entry **bins;
} st_table;

typedef struct _st_table_entry {
	unsigned int hash;
	long key;
	long record;
	struct _st_table_entry *next;
} st_table_entry;

typedef struct _st_hash_type {
	int (*compare)(void *v1, void *v2);
	int (*hash)(void *v);
	int (*print)(void *v);
} st_hash_type;


extern st_table_entry *st_lookup(st_table *table, long key, long *ret);
extern int st_add_direct(st_table *table, long key, long val);
extern int st_add(st_table *table, long key, long val);
extern int st_rewrite(st_table *table, long key, long val);
extern int st_del(st_table *table, long key);


extern st_table *st_init_numtable( void );
extern st_table *st_init_strtable( void );
extern void print_hash(st_table *table);

#endif
