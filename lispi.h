
#ifndef LISPI_H
#define LISPI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "objects.h"

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef char INT8;
typedef short INT16;
typedef int INT32;


#define TRUE 1 // C program level TRUE, FALSE
#define FALSE 0

#define Calloc(n,s) ((s <= 0) ? NULL : (calloc((n), (s))))
#define Malloc(s) ((s <= 0) ? NULL : (malloc((s))))
#define OBJ_Malloc(obj_type) (obj_type *)malloc( sizeof(obj_type) )
#define OBJN_Malloc(obj_type,s) (obj_type *)( Calloc(sizeof(obj_type),(s)) )

#define FREE(p) free(p)
#define FREE_ENV(e) 

#define Plist(l) print_list("=>", (VALUE)l)

// for Debug Print
#ifdef Debug
#define Denv(env) D(), print_env(env)
#define Dlist(l) D(), print_list("", (VALUE)l)
#define D() printf("%s %s:%d:\n",__FILE__,__func__, __LINE__)
#define Ds(str) printf("%s %s:%d: %s\n",__FILE__, __func__, __LINE__, str)
#define Dexit(i) printf("%s %s:%d:\n",__FILE__,__func__, __LINE__), exit(i)
#define DUMPSTR(str) {\
        unsigned char *p;\
        for(p = str ;  *p != '\0' ; p++) { printf("%x, ",p); }\
        printf("\n");\
    }
#else
#define Denv(env)
#define Dlist(l)
#define D()
#define Ds(str)
#define Dexit(i) exit(i)
#define DUMPSTR(str)
#endif



#endif

