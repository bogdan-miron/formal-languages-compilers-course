#ifndef RG_H
#define RG_H

#include <stdio.h>
#include "fa.h"

#define MAX_PRODS 320

/* one production: lhs -> sym rhs.  sym -1 means epsilon, rhs -1 means no nonterminal */
typedef struct {
    int lhs, sym, rhs;
} Prod;

/* right linear regular grammar, the nonterminals are the states of the automaton */
typedef struct {
    const FA *fa;                    /* gives the names of the terminals */
    int n;                           /* number of nonterminals */
    char nt[MAX_STATES + 1][NAME_LEN];
    int start;
    int nprods;
    Prod prods[MAX_PRODS];
} RG;

/* the transformation of the lab: finite automaton to regular grammar */
void rg_from_fa(const FA *fa, RG *g);

void rg_print(const RG *g, FILE *out);

/* checks a word using only the productions */
int rg_accepts(const RG *g, const char *word);

#endif
