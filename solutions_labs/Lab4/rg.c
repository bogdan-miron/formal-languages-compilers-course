#include <stdio.h>
#include <string.h>
#include "rg.h"

static void add(RG *g, int lhs, int sym, int rhs)
{
    g->prods[g->nprods].lhs = lhs;
    g->prods[g->nprods].sym = sym;
    g->prods[g->nprods].rhs = rhs;
    g->nprods++;
}

/* for every transition p -a-> q the grammar gets  P -> a Q,
   and if q is a final state it also gets  P -> a.
   if the start state is final the grammar gets  S -> epsilon.
   when something also goes back into a final start state, a new start
   symbol S' is used, so that S' does not appear on any right side */
void rg_from_fa(const FA *fa, RG *g)
{
    memset(g, 0, sizeof *g);
    g->fa = fa;
    g->n = fa->nstates;
    for (int i = 0; i < fa->nstates; i++) strcpy(g->nt[i], fa->states[i]);
    g->start = fa->start;

    int comes_back = 0;
    for (int t = 0; t < fa->ntrans; t++)
        if (fa->trans[t].to == fa->start) comes_back = 1;

    int new_start = fa->final[fa->start] && comes_back;
    if (new_start) {
        strcpy(g->nt[g->n], "S'");
        g->start = g->n;
        g->n++;
    }

    for (int p = 0; p < fa->nstates; p++) {
        for (int t = 0; t < fa->ntrans; t++) {
            if (fa->trans[t].from != p) continue;
            add(g, p, fa->trans[t].sym, fa->trans[t].to);
            if (fa->final[fa->trans[t].to]) add(g, p, fa->trans[t].sym, -1);
        }
    }

    if (new_start) {
        /* S' gets a copy of every production of the old start symbol */
        int old = g->nprods;
        for (int k = 0; k < old; k++)
            if (g->prods[k].lhs == fa->start) add(g, g->start, g->prods[k].sym, g->prods[k].rhs);
    }
    if (fa->final[fa->start]) add(g, g->start, -1, -1);
}

static void print_alternative(const RG *g, const Prod *p, FILE *out)
{
    char text[64];
    if (p->sym < 0) {
        fprintf(out, "epsilon");
        return;
    }
    fa_symbol_text(g->fa, p->sym, text, sizeof text);
    fprintf(out, "%s", text);
    if (p->rhs >= 0) fprintf(out, " %s", g->nt[p->rhs]);
}

void rg_print(const RG *g, FILE *out)
{
    const FA *fa = g->fa;
    char text[512];

    fprintf(out, "regular grammar for %s\n", fa->name);
    fprintf(out, "N = {");
    for (int i = 0; i < g->n; i++) fprintf(out, "%s %s", i ? "," : "", g->nt[i]);
    fprintf(out, " }\n");

    fprintf(out, "T = {");
    int first = 1;
    for (int s = 0; s < fa->nsyms; s++) {
        int used = 0;
        for (int k = 0; k < g->nprods; k++)
            if (g->prods[k].sym == s) used = 1;
        if (!used) continue;
        fa_symbol_text(fa, s, text, sizeof text);
        fprintf(out, "%s %s", first ? "" : ",", text);
        first = 0;
    }
    fprintf(out, " }\n");
    fprintf(out, "S = %s\n", g->nt[g->start]);

    for (int s = 0; s < fa->nsyms; s++) {
        if (fa->syms[s].count == 1) continue;
        fa_set_text(&fa->syms[s], text, sizeof text);
        fprintf(out, "%s stands for one of: %s\n", fa->syms[s].name, text);
    }

    fprintf(out, "P:\n");
    /* start symbol first, then the others in order */
    for (int pass = 0; pass < g->n; pass++) {
        int nt = pass == 0 ? g->start : (pass - 1 < g->start ? pass - 1 : pass);
        int count = 0;
        for (int k = 0; k < g->nprods; k++) {
            if (g->prods[k].lhs != nt) continue;
            fprintf(out, count == 0 ? "  %s -> " : " | ", g->nt[nt]);
            print_alternative(g, &g->prods[k], out);
            count++;
        }
        if (count) fprintf(out, "\n");
    }
}

static int has_epsilon(const RG *g, int nt)
{
    for (int k = 0; k < g->nprods; k++)
        if (g->prods[k].lhs == nt && g->prods[k].sym < 0) return 1;
    return 0;
}

/* keeps the set of nonterminals the word can be in after each character */
int rg_accepts(const RG *g, const char *word)
{
    int cur[MAX_STATES + 1] = {0};
    int next[MAX_STATES + 1];
    int len = (int)strlen(word);

    cur[g->start] = 1;
    if (len == 0) return has_epsilon(g, g->start);

    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)word[i];
        memset(next, 0, sizeof next);
        for (int k = 0; k < g->nprods; k++) {
            const Prod *p = &g->prods[k];
            if (!cur[p->lhs] || p->sym < 0 || !g->fa->syms[p->sym].has[c]) continue;
            if (p->rhs >= 0) next[p->rhs] = 1;
            else if (i == len - 1) return 1;
        }
        memcpy(cur, next, sizeof cur);
    }
    for (int a = 0; a < g->n; a++)
        if (cur[a] && has_epsilon(g, a)) return 1;
    return 0;
}
