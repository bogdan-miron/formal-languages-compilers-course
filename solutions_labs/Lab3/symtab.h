#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdio.h>

/* symbol table, a hash table with chaining.
   it keeps the identifiers and constants of the program, and also the
   token list from tokens.txt */

#define ST_SIZE 37

typedef struct Entry {
    char *key;           /* the text of the symbol */
    int value;           /* class of the symbol, or code of the token */
    struct Entry *next;  /* next one in the same bucket */
} Entry;

typedef struct {
    Entry *bucket[ST_SIZE];
    int count;
} SymTable;

/* position of a symbol: bucket and index in the chain */
typedef struct {
    int bucket;
    int index;
} StPos;

SymTable *st_create(void);
void st_free(SymTable *t);

/* adds key if it is not there yet and returns its position.
   added is set to 1 if it was new, 0 if it was already there */
StPos st_insert(SymTable *t, const char *key, int value, int *added);

/* finds key, returns NULL if it is not there */
Entry *st_lookup(const SymTable *t, const char *key, StPos *pos);

/* prints all the entries, bucket by bucket */
void st_dump(const SymTable *t, FILE *out);

#endif
