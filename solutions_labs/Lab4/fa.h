#ifndef FA_H
#define FA_H

#include <stdio.h>

#define NAME_LEN 32
#define MAX_STATES 32
#define MAX_SYMBOLS 16
#define MAX_TRANS 128

/* a symbol of the alphabet is a set of characters, for example letter = a-z A-Z.
   a symbol with one character is just that character. */
typedef struct {
    char name[NAME_LEN];
    unsigned char has[256];   /* has[c] is 1 if character c is in the symbol */
    int count;                /* how many characters */
    int first;                /* the character, when count is 1 */
} Symbol;

typedef struct {
    int from, sym, to;
} Trans;

/* deterministic finite automaton */
typedef struct {
    char name[NAME_LEN];
    int nstates;
    char states[MAX_STATES][NAME_LEN];
    int nsyms;
    Symbol syms[MAX_SYMBOLS];
    int sym_of[256];                     /* symbol of a character, or -1 */
    int ntrans;
    Trans trans[MAX_TRANS];
    int delta[MAX_STATES][MAX_SYMBOLS];  /* next state, or -1 */
    int start;
    int final[MAX_STATES];
} FA;

/* returns 0 if the file is ok, otherwise prints the problem and returns 1 */
int fa_load(FA *fa, const char *path);

int fa_accepts(const FA *fa, const char *word);

/* like fa_accepts, but prints the states it goes through */
int fa_trace(const FA *fa, const char *word, FILE *out);

void fa_print(const FA *fa, FILE *out);

/* graphviz description of the automaton */
void fa_dot(const FA *fa, FILE *out);

/* text of a symbol: its name, or the quoted character if it has only one */
void fa_symbol_text(const FA *fa, int sym, char *buf, size_t size);

/* the characters of a symbol written as ranges, like A-Z a-z */
void fa_set_text(const Symbol *s, char *buf, size_t size);

#endif
