#include <stdlib.h>
#include <string.h>
#include "pif.h"

void pif_init(Pif *p) {
    p->items = NULL;
    p->size = 0;
    p->capacity = 0;
}

void pif_add(Pif *p, const char *text, int code, const StPos *pos) {
    if (p->size == p->capacity) {
        p->capacity = p->capacity ? p->capacity * 2 : 64;
        p->items = realloc(p->items, p->capacity * sizeof(PifItem));
        if (!p->items) { fprintf(stderr, "out of memory\n"); exit(1); }
    }
    PifItem *it = &p->items[p->size++];
    it->text = malloc(strlen(text) + 1);
    if (!it->text) { fprintf(stderr, "out of memory\n"); exit(1); }
    strcpy(it->text, text);
    it->code = code;
    it->has_pos = pos != NULL;
    if (pos) it->pos = *pos;
}

void pif_write(const Pif *p, FILE *out) {
    fprintf(out, "%-28s %4s  %s\n", "token", "code", "ST position");
    for (int i = 0; i < p->size; i++) {
        const PifItem *it = &p->items[i];
        if (it->has_pos)
            fprintf(out, "%-28s %4d  (%d,%d)\n", it->text, it->code, it->pos.bucket, it->pos.index);
        else
            fprintf(out, "%-28s %4d  -\n", it->text, it->code);
    }
}

void pif_free(Pif *p) {
    for (int i = 0; i < p->size; i++) free(p->items[i].text);
    free(p->items);
    p->items = NULL;
    p->size = p->capacity = 0;
}
