#include <stdlib.h>
#include <string.h>
#include "symtab.h"

static unsigned hash(const char *s) {
    unsigned h = 0;
    while (*s)
        h = (h * 31 + (unsigned char)*s++) % ST_SIZE;
    return h;
}

static char *copy(const char *s) {
    char *c = malloc(strlen(s) + 1);
    if (!c) { fprintf(stderr, "out of memory\n"); exit(1); }
    strcpy(c, s);
    return c;
}

SymTable *st_create(void) {
    SymTable *t = calloc(1, sizeof(SymTable));
    if (!t) { fprintf(stderr, "out of memory\n"); exit(1); }
    return t;
}

void st_free(SymTable *t) {
    for (int i = 0; i < ST_SIZE; i++) {
        Entry *e = t->bucket[i];
        while (e) {
            Entry *next = e->next;
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(t);
}

Entry *st_lookup(const SymTable *t, const char *key, StPos *pos) {
    unsigned b = hash(key);
    int index = 0;
    for (Entry *e = t->bucket[b]; e; e = e->next, index++) {
        if (strcmp(e->key, key) == 0) {
            if (pos) { pos->bucket = (int)b; pos->index = index; }
            return e;
        }
    }
    return NULL;
}

StPos st_insert(SymTable *t, const char *key, int value, int *added) {
    StPos pos;
    if (st_lookup(t, key, &pos)) {
        if (added) *added = 0;
        return pos;
    }
    Entry *e = malloc(sizeof(Entry));
    if (!e) { fprintf(stderr, "out of memory\n"); exit(1); }
    e->key = copy(key);
    e->value = value;
    e->next = NULL;

    unsigned b = hash(key);
    int index = 0;
    Entry **link = &t->bucket[b];
    while (*link) { link = &(*link)->next; index++; }
    *link = e;
    t->count++;

    pos.bucket = (int)b;
    pos.index = index;
    if (added) *added = 1;
    return pos;
}

void st_dump(const SymTable *t, FILE *out) {
    for (int i = 0; i < ST_SIZE; i++) {
        int index = 0;
        for (Entry *e = t->bucket[i]; e; e = e->next, index++)
            fprintf(out, "(%d,%d)  %s\n", i, index, e->key);
    }
}
