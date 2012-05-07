#include "lispi.h"
#include "objects.h"
#include "hash.h"



// -------------------- hash ----------------
static int stringcmp(void *s1, void *s2);
static int strhash(void *str);
static int str_print(void *str);
static int numcmp(void *n, void *m);
static int numhash(void *n);
static int num_print(void *n);

// key type of hash table
static st_hash_type hash_type_str = { stringcmp, strhash, str_print };
static st_hash_type hash_type_num = { numcmp, numhash, num_print };


static int stringcmp(void *s1, void *s2)
{
    return li_strcmp((const char *)s1, (const char *)s2);
}

static int strhash(void *str)
{
    register int val = 0;
	char c;
    char *str2 = (char *)str;
	while ((c = *str2++) != '\0') {
		val = val*997 + c;
	}
	return val + (val>>5);
}

static int str_print(void *str)
{
	println_str((char *)str);
	return 0;
}

static int numcmp(void *n, void *m)
{
	return (long)n != (long)m;
}
static int numhash(void *n)
{
	return (long)n;
}
static int num_print(void *n)
{
	println_int((long)n);
	return 0;
}

st_table *st_init_table_with_size(st_hash_type *type, int size)
{
	st_table *table = OBJ_Malloc(st_table);
	table->type = type;
	table->num_entries = 0;
	table->num_bins=size;
	table->bins = (st_table_entry**)Calloc(size, sizeof(st_table_entry*));
	return table;
}

st_table *st_init_numtable()
{
	return st_init_table_with_size(&hash_type_num, 0);
}
st_table *st_init_strtable()
{
	return st_init_table_with_size(&hash_type_str, 0);
}


static void rehash(st_table *table)
{
	int new_num_bins = table->num_bins+NUM_INCREMENT_REHASH;
	st_table_entry **new_bins = (st_table_entry**)Calloc(new_num_bins, sizeof(st_table_entry*));
	int i;

	for(i=0 ; i < new_num_bins ; i++) {
		new_bins[i] = NULL;
	}

	for(i=0 ; i < table->num_bins ; i++) {
		st_table_entry *next;
		st_table_entry *entry = table->bins[i];
		while(entry) {
			int pos = entry->hash % new_num_bins;
			next = entry->next;
			entry->next = new_bins[pos];
			new_bins[pos] = entry;
			entry = next;
		}
	}
	li_free(table->bins);
	table->bins = new_bins;
	table->num_bins = new_num_bins;
}

#define do_hash(t,k) (unsigned int)t->type->hash((void *)(k))
#define EQUAL(t,k1,k2) ((k1) == (k2) || t->type->compare((void*)(k1),(void*)(k2)) == 0)
int st_add_direct(st_table *table, long key, long val)
{
	long hash, pos;
	st_table_entry *new_entry;

	hash = do_hash(table, key);
	if(0 == table->num_bins || table->num_entries / table->num_bins > HASH_MAX_DENCITY) {
		rehash(table);
	}
	new_entry = (st_table_entry *)Malloc(sizeof(st_table_entry));
	pos = do_hash(table, key) % table->num_bins;
	new_entry->hash = hash;
	new_entry->key = key;
	new_entry->record = val;
	new_entry->next = table->bins[pos];
	table->bins[pos] = new_entry;
	table->num_entries++;

	return 1;
}

st_table_entry *st_lookup(st_table *table, long key, long *ret)
{
	long pos;
	st_table_entry *p;
	if(0 == table->num_entries) return NULL;

	pos = do_hash(table,key) % table->num_bins;
	p = table->bins[pos];
	while(p) {
		if(EQUAL(table, key, p->key)){
			*ret = p->record;
			break;
		}
		p = p->next;
	}
	return p;
}

/**
 * Add entry into TABLE.
 * if already entried tye KEY, rewrite record by VAL.
 */
int st_rewrite(st_table *table, long key, long val)
{
	long record;
	int ret = 0;
	st_table_entry *entry; 

	entry = st_lookup(table, key, &record);
	if(!entry) {
		ret = st_add_direct(table, key, val);
	} else if(record != val) {
		entry->record = val;
	}
	return ret;
}

int st_add(st_table *table, long key, long val)
{
	long record;
	int ret = 0;

	if(!st_lookup(table, key, &record)) {
		ret = st_add_direct(table, key, val);
	}
	return ret;
}

int st_del(st_table *table, long key)
{
	long record;
	int ret = 0;
	st_table_entry *entry; 

	entry = st_lookup(table, key, &record);
	if(entry) {
	}
	return ret;
}

void print_hash(st_table *table)
{
	int i;
	for(i = 0 ; i < table->num_bins ; i++) {
		st_table_entry *p = table->bins[i];
		while(p) {
			//printf("hash:0x%x, val:%ld ",p->hash, (long)p->record);
			table->type->print((void *)(p->key));
			p = p->next;
		}
	}
}


