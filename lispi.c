
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lispi.h"

unsigned int li_strlen(const char *s)
{
    return (unsigned int)strlen(s);
}

char *li_strchr( const char *s, int c)
{
    return strchr(s, c);
}

int li_isalpha(int c)
{
    return isalpha(c);
}
int li_isspace(int c)
{
    return isspace(c);
}
int li_isdigit(int c)
{
    return isdigit(c);
}

void *li_malloc(int size)
{
    return malloc((size_t)size);
}

void li_free(void *p)
{
    free(p);
}

void *li_calloc(int n, int s)
{
    char *p;
    char *ret = li_malloc(n*s);

    for (p = ret ; p != ret+(n*s) ; p++) {
        *p = 0;
    }

    return ret;
}

int li_strcmp(const char *s1, const char *s2)
{
    register const unsigned char *ss1, *ss2;
    for (ss1 = (const unsigned char*)s1, ss2 = (const unsigned char*)s2;
        *ss1 == *ss2 && *ss1 != '\0';
        ss1++, ss2++) {
    }
    return *ss1 - *ss2;
}

void print_str(const char *str)
{
    printf("%s",str);
}

void println_str(const char *str)
{

    printf("%s",str);
    printf("\n");
}
void print_int(int n)
{

    printf("%d", n);
}
void println_int(int n)
{

    printf("%d", n);
    printf("\n");
}

void get_line(char *buf, int size)
{
	fgets(buf, size, stdin);
}

void li_exit(int n)
{
    exit(n);
}

