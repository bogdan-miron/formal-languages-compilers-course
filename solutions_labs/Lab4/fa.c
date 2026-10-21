#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fa.h"

#define MAX_WORDS 80

/* ---------- reading the automaton file ---------- */

static int problem(const char *path, int line, const char *what, const char *arg)
{
    fprintf(stderr, "%s, line %d: %s%s%s\n", path, line, what, arg[0] ? " " : "", arg);
    return 1;
}

static int find_state(const FA *fa, const char *name)
{
    for (int i = 0; i < fa->nstates; i++)
        if (strcmp(fa->states[i], name) == 0) return i;
    return -1;
}

static int find_symbol(const FA *fa, const char *name)
{
    for (int i = 0; i < fa->nsyms; i++)
        if (strcmp(fa->syms[i].name, name) == 0) return i;
    return -1;
}

/* a name is letters, digits and underscores */
static int good_name(const char *s)
{
    if (s[0] == '\0' || strlen(s) >= NAME_LEN) return 0;
    for (; *s; s++)
        if (!isalnum((unsigned char)*s) && *s != '_') return 0;
    return 1;
}

/* splits a line into words, returns how many, or -1 if there are too many */
static int split(char *line, char **words)
{
    int n = 0;
    char *w = strtok(line, " \t\r\n");
    while (w) {
        if (n == MAX_WORDS) return -1;
        words[n++] = w;
        w = strtok(NULL, " \t\r\n");
    }
    return n;
}

/* reads one character, \s is a space and \\ and \- are the characters themselves.
   returns -1 for a bad escape */
static int read_char(const char **s)
{
    unsigned char c = (unsigned char)**s;
    (*s)++;
    if (c != '\\') return c;
    c = (unsigned char)**s;
    if (c == 's') { (*s)++; return ' '; }
    if (c == '\\' || c == '-') { (*s)++; return c; }
    return -1;
}

/* adds the characters of one item (a character or a range like a-z) to a symbol */
static int add_item(FA *fa, int sym, const char *item)
{
    const char *p = item;
    int lo = read_char(&p);
    int hi = lo;
    if (lo < 0) return 1;
    if (*p == '-' && p[1] != '\0') {
        p++;
        hi = read_char(&p);
        if (hi < 0) return 1;
    }
    if (*p != '\0' || hi < lo) return 1;
    for (int c = lo; c <= hi; c++) {
        if (fa->sym_of[c] != -1 && fa->sym_of[c] != sym) return 2;
        if (!fa->syms[sym].has[c]) {
            fa->syms[sym].has[c] = 1;
            fa->syms[sym].count++;
            fa->syms[sym].first = c;
        }
        fa->sym_of[c] = sym;
    }
    return 0;
}

int fa_load(FA *fa, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cannot open '%s'\n", path);
        return 1;
    }

    memset(fa, 0, sizeof *fa);
    memset(fa->sym_of, -1, sizeof fa->sym_of);
    memset(fa->delta, -1, sizeof fa->delta);
    fa->start = -1;

    char line[512];
    char *w[MAX_WORDS];
    int line_no = 0, bad = 0, has_name = 0;

    while (!bad && fgets(line, sizeof line, f)) {
        line_no++;
        if (!strchr(line, '\n') && !feof(f)) {
            bad = problem(path, line_no, "line is too long", "");
            break;
        }
        int n = split(line, w);
        if (n < 0) { bad = problem(path, line_no, "too many words on the line", ""); break; }
        if (n == 0 || w[0][0] == '#') continue;

        if (strcmp(w[0], "name") == 0) {
            if (n != 2 || !good_name(w[1])) bad = problem(path, line_no, "expected: name <name>", "");
            else { strcpy(fa->name, w[1]); has_name = 1; }

        } else if (strcmp(w[0], "symbol") == 0) {
            if (n < 3 || !good_name(w[1])) {
                bad = problem(path, line_no, "expected: symbol <name> <characters>", "");
            } else if (find_symbol(fa, w[1]) >= 0 || find_state(fa, w[1]) >= 0) {
                bad = problem(path, line_no, "name is already used:", w[1]);
            } else if (fa->nsyms == MAX_SYMBOLS) {
                bad = problem(path, line_no, "too many symbols", "");
            } else {
                int sym = fa->nsyms++;
                strcpy(fa->syms[sym].name, w[1]);
                for (int i = 2; i < n && !bad; i++) {
                    int r = add_item(fa, sym, w[i]);
                    if (r == 1) bad = problem(path, line_no, "bad character or range:", w[i]);
                    if (r == 2) bad = problem(path, line_no, "a character is in two symbols:", w[i]);
                }
            }

        } else if (strcmp(w[0], "states") == 0) {
            if (n < 2) bad = problem(path, line_no, "expected: states <names>", "");
            for (int i = 1; i < n && !bad; i++) {
                if (!good_name(w[i]) || strcmp(w[i], "name") == 0 || strcmp(w[i], "symbol") == 0 ||
                    strcmp(w[i], "states") == 0 || strcmp(w[i], "start") == 0 ||
                    strcmp(w[i], "final") == 0)
                    bad = problem(path, line_no, "bad state name:", w[i]);
                else if (find_state(fa, w[i]) >= 0 || find_symbol(fa, w[i]) >= 0)
                    bad = problem(path, line_no, "name is already used:", w[i]);
                else if (fa->nstates == MAX_STATES)
                    bad = problem(path, line_no, "too many states", "");
                else
                    strcpy(fa->states[fa->nstates++], w[i]);
            }

        } else if (strcmp(w[0], "start") == 0) {
            if (n != 2 || find_state(fa, w[1]) < 0) bad = problem(path, line_no, "start must be a declared state", "");
            else if (fa->start >= 0) bad = problem(path, line_no, "start is given twice", "");
            else fa->start = find_state(fa, w[1]);

        } else if (strcmp(w[0], "final") == 0) {
            if (n < 2) bad = problem(path, line_no, "expected: final <states>", "");
            for (int i = 1; i < n && !bad; i++) {
                int s = find_state(fa, w[i]);
                if (s < 0) bad = problem(path, line_no, "unknown state:", w[i]);
                else fa->final[s] = 1;
            }

        } else {
            /* a transition: <state> <symbol> <state> */
            int from = n == 3 ? find_state(fa, w[0]) : -1;
            int sym = n == 3 ? find_symbol(fa, w[1]) : -1;
            int to = n == 3 ? find_state(fa, w[2]) : -1;
            if (n != 3) bad = problem(path, line_no, "expected a transition: <state> <symbol> <state>", "");
            else if (from < 0) bad = problem(path, line_no, "unknown state:", w[0]);
            else if (sym < 0) bad = problem(path, line_no, "unknown symbol:", w[1]);
            else if (to < 0) bad = problem(path, line_no, "unknown state:", w[2]);
            else if (fa->delta[from][sym] >= 0) bad = problem(path, line_no, "second transition from the same state on", w[1]);
            else if (fa->ntrans == MAX_TRANS) bad = problem(path, line_no, "too many transitions", "");
            else {
                fa->delta[from][sym] = to;
                fa->trans[fa->ntrans].from = from;
                fa->trans[fa->ntrans].sym = sym;
                fa->trans[fa->ntrans].to = to;
                fa->ntrans++;
            }
        }
    }
    fclose(f);
    if (bad) return 1;

    if (!has_name) return problem(path, line_no, "the file has no name line", "");
    if (fa->nstates == 0) return problem(path, line_no, "the file has no states", "");
    if (fa->start < 0) return problem(path, line_no, "the file has no start state", "");
    return 0;
}

/* ---------- running the automaton ---------- */

static int run(const FA *fa, const char *word, FILE *out)
{
    int state = fa->start;
    if (out) fprintf(out, "%s", fa->states[state]);
    for (int i = 0; word[i]; i++) {
        unsigned char c = (unsigned char)word[i];
        int sym = fa->sym_of[c];
        int next = sym < 0 ? -1 : fa->delta[state][sym];
        if (next < 0) {
            if (out) {
                if (sym < 0) fprintf(out, "\nstops: '%c' is not in the alphabet\n", c);
                else fprintf(out, "\nstops: no transition from %s on '%c'\n", fa->states[state], c);
            }
            return 0;
        }
        if (out) fprintf(out, " -'%c'-> %s", c, fa->states[next]);
        state = next;
    }
    if (out) fprintf(out, "\n");
    return fa->final[state];
}

int fa_accepts(const FA *fa, const char *word)
{
    return run(fa, word, NULL);
}

int fa_trace(const FA *fa, const char *word, FILE *out)
{
    return run(fa, word, out);
}

/* ---------- printing ---------- */

static void put(char *buf, size_t size, size_t *len, const char *s)
{
    size_t n = strlen(s);
    if (*len + n + 1 > size) return;
    memcpy(buf + *len, s, n + 1);
    *len += n;
}

static void char_text(int c, char *out)
{
    if (c == ' ') strcpy(out, "space");
    else { out[0] = (char)c; out[1] = '\0'; }
}

void fa_set_text(const Symbol *s, char *buf, size_t size)
{
    size_t len = 0;
    char a[8], b[8];
    buf[0] = '\0';
    int c = 0;
    while (c < 256) {
        if (!s->has[c]) { c++; continue; }
        int end = c;
        while (end + 1 < 256 && s->has[end + 1]) end++;
        if (len) put(buf, size, &len, " ");
        if (end - c >= 2) {
            char_text(c, a);
            char_text(end, b);
            put(buf, size, &len, a);
            put(buf, size, &len, "-");
            put(buf, size, &len, b);
        } else {
            for (int k = c; k <= end; k++) {
                if (k > c) put(buf, size, &len, " ");
                char_text(k, a);
                put(buf, size, &len, a);
            }
        }
        c = end + 1;
    }
}

void fa_symbol_text(const FA *fa, int sym, char *buf, size_t size)
{
    const Symbol *s = &fa->syms[sym];
    if (s->count == 1) {
        if (s->first == '"') snprintf(buf, size, "'\"'");
        else snprintf(buf, size, "\"%c\"", s->first);
    } else {
        snprintf(buf, size, "%s", s->name);
    }
}

void fa_print(const FA *fa, FILE *out)
{
    char text[512];

    fprintf(out, "automaton: %s\n", fa->name);
    fprintf(out, "states:   ");
    for (int i = 0; i < fa->nstates; i++) fprintf(out, " %s", fa->states[i]);
    fprintf(out, "\nstart:     %s\n", fa->states[fa->start]);
    fprintf(out, "final:    ");
    for (int i = 0; i < fa->nstates; i++)
        if (fa->final[i]) fprintf(out, " %s", fa->states[i]);
    fprintf(out, "\nalphabet:\n");
    for (int i = 0; i < fa->nsyms; i++) {
        fa_set_text(&fa->syms[i], text, sizeof text);
        fprintf(out, "  %s = %s\n", fa->syms[i].name, text);
    }

    int name_w = 0, col_w = 0;
    for (int i = 0; i < fa->nstates; i++)
        if ((int)strlen(fa->states[i]) > name_w) name_w = (int)strlen(fa->states[i]);
    col_w = name_w;
    for (int i = 0; i < fa->nsyms; i++)
        if ((int)strlen(fa->syms[i].name) > col_w) col_w = (int)strlen(fa->syms[i].name);
    col_w += 2;

    fprintf(out, "transitions (-> start, * final, - no transition):\n");
    fprintf(out, "   %-*s", name_w + 2, "");
    for (int j = 0; j < fa->nsyms; j++) fprintf(out, "%-*s", col_w, fa->syms[j].name);
    fprintf(out, "\n");
    for (int i = 0; i < fa->nstates; i++) {
        const char *mark = "   ";
        if (i == fa->start && fa->final[i]) mark = "->*";
        else if (i == fa->start) mark = "-> ";
        else if (fa->final[i]) mark = "*  ";
        fprintf(out, "%s%-*s", mark, name_w + 2, fa->states[i]);
        for (int j = 0; j < fa->nsyms; j++) {
            int to = fa->delta[i][j];
            fprintf(out, "%-*s", col_w, to < 0 ? "-" : fa->states[to]);
        }
        fprintf(out, "\n");
    }
}

/* ---------- graphviz ---------- */

static void dot_label(const FA *fa, int sym, FILE *out)
{
    const Symbol *s = &fa->syms[sym];
    if (s->count != 1) fprintf(out, "%s", s->name);
    else if (s->first == ' ') fprintf(out, "space");
    else if (s->first == '"') fprintf(out, "\\\"");
    else if (s->first == '\\') fprintf(out, "\\\\");
    else fprintf(out, "%c", s->first);
}

void fa_dot(const FA *fa, FILE *out)
{
    fprintf(out, "digraph %s {\n", fa->name);
    fprintf(out, "  rankdir=LR;\n  node [shape=circle];\n");
    for (int i = 0; i < fa->nstates; i++)
        if (fa->final[i]) fprintf(out, "  %s [shape=doublecircle];\n", fa->states[i]);
    fprintf(out, "  start [shape=point];\n  start -> %s;\n", fa->states[fa->start]);

    /* transitions with the same two states are drawn as one arrow */
    for (int i = 0; i < fa->ntrans; i++) {
        int seen = 0;
        for (int j = 0; j < i; j++)
            if (fa->trans[j].from == fa->trans[i].from && fa->trans[j].to == fa->trans[i].to) seen = 1;
        if (seen) continue;
        fprintf(out, "  %s -> %s [label=\"", fa->states[fa->trans[i].from], fa->states[fa->trans[i].to]);
        int first = 1;
        for (int j = i; j < fa->ntrans; j++) {
            if (fa->trans[j].from != fa->trans[i].from || fa->trans[j].to != fa->trans[i].to) continue;
            if (!first) fprintf(out, ", ");
            dot_label(fa, fa->trans[j].sym, out);
            first = 0;
        }
        fprintf(out, "\"];\n");
    }
    fprintf(out, "}\n");
}
