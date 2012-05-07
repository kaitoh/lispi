
#ifndef LISPI_H
#define LISPI_H

#ifndef NULL
#define NULL 0
#endif

extern void *li_malloc(int size);
extern void li_free(void *p);
extern void *li_calloc(int n, int s);
extern int li_strcmp(const char *s1, const char *s2);
extern void li_exit(int n);
extern int li_isalpha(int c);
extern int li_isspace(int c);
extern int li_isdigit(int c);
extern char *li_strchr( const char *s, int c);
extern unsigned int li_strlen(const char *s);

extern void print_str(const char *str);
extern void println_str(const char *str);
extern void print_int(int n);
extern void println_int(int n);
extern void get_line(char *buf, int size);

#define TRUE 1 // C program level TRUE, FALSE
#define FALSE 0

#define Calloc(n,s) ((s <= 0) ? NULL : (li_calloc((n), (s))))
#define Malloc(s) ((s <= 0) ? NULL : (li_malloc((s))))
#define OBJ_Malloc(obj_type) (obj_type *)li_malloc( sizeof(obj_type) )

#define FREE(p) li_free(p)
#define FREE_ENV(e) 


#define Plist(l) print_list("=>", (VALUE)l)

// for Debug Print
#ifdef Debug
#define Denv(env) D(), print_env(env)
#define Dlist(l) D(), print_list("", (VALUE)l)
#define D() print_str(__FILE__" "), print_str(__func__), print_str(":"),  print_int(__LINE__), print_str(":\n")
#define Ds(str) print_str(__FILE__" "), print_str(__func__), print_str(":"),  print_int(__LINE__), print_str(str), print_str(":\n")
#define Dexit(i) print_str(__FILE__" "), print_str(__func__), print_str(":"),  print_int(__LINE__), print_str(":\n"), li_exit(i)
#else
#define Denv(env)
#define Dlist(l)
#define D()
#define Ds(str)
#define Dexit(i) li_exit(i)
#endif



#endif

