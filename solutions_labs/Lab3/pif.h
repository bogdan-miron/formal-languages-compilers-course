#ifndef PIF_H
#define PIF_H

#include <stdio.h>
#include "symtab.h"

/* the pif: the tokens of the program in the order they appear.
   identifiers and constants also keep their position in the st,
   the other tokens have no position */

typedef struct {
    char *text;
    int code;
    int has_pos;
    StPos pos;
} PifItem;

typedef struct {
    PifItem *items;
    int size;
    int capacity;
} Pif;

void pif_init(Pif *p);
void pif_add(Pif *p, const char *text, int code, const StPos *pos);
void pif_write(const Pif *p, FILE *out);
void pif_free(Pif *p);

#endif
