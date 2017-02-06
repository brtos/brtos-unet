/*
 * string_lib.h
 *
 */

#ifndef STRING_LIB_H_
#define STRING_LIB_H_

#define _restrict  restrict

void *memchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void * _restrict s1, const void * _restrict s2, size_t n);
void *memmove(void *s1, const void *s2, size_t n);
void *memset(void *s, int c, size_t n);
char *strcat(char * _restrict s1,const char * _restrict s2);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char *strcpy(char * _restrict s1,const char * _restrict s2);
size_t strcspn(const char *s1, const char *s2);
size_t strlen(const char *s);
char *strncat(char * _restrict s1,const char * _restrict s2, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char * _restrict s1,const char * _restrict s2, size_t n);
char *strpbrk(const char *s1, const char *s2);
char *strrchr(const char *s, int c);
size_t strspn(const char *s1, const char *s2);
char *strstr(const char *s1, const char *s2);
char *strtok(char * _restrict s1,const char * _restrict s2);

/* not implemented !!! */
size_t strxfrm(char * _restrict s1,const char * _restrict s2, size_t n);
int strcoll(const char *s1, const char *s2);
char *strerror(int errnum);


#endif /* LIBS_STRING_LIB_H_ */
