#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

SymTable *symbols;
SymTable *tokens;
Pif pif;
FILE *errors_out;
int error_count;
int code_identifier, code_number, code_string, code_address;

static void fatal(const char *fmt, const char *arg) {
    fprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    exit(2);
}

/* Reads the token list. Each line is "<code> <token>". */
static void load_tokens(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) fatal("cannot open token list '%s'", path);

    char line[256];
    int line_no = 0;
    while (fgets(line, sizeof line, f)) {
        line_no++;
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;

        char *end;
        long code = strtol(line, &end, 10);
        if (end == line || *end != ' ' || end[1] == '\0') {
            fprintf(stderr, "%s, line %d: expected '<code> <token>'\n", path, line_no);
            exit(2);
        }
        const char *text = end + 1;
        int added;
        st_insert(tokens, text, (int)code, &added);
        if (!added) {
            fprintf(stderr, "%s, line %d: token '%s' is listed twice\n", path, line_no, text);
            exit(2);
        }
    }
    fclose(f);

    struct { const char *name; int *code; } classes[] = {
        { "identifier", &code_identifier }, { "number", &code_number },
        { "string", &code_string },         { "address", &code_address },
    };
    for (int i = 0; i < 4; i++) {
        Entry *e = st_lookup(tokens, classes[i].name, NULL);
        if (!e) fatal("token list has no entry for '%s'", classes[i].name);
        *classes[i].code = e->value;
    }
}

/* Output files are <prefix>_PIF.out, <prefix>_ST.out and <prefix>_errors.out.
   The default prefix is the name of the program without its extension. */
static void make_prefix(char *buf, size_t size, const char *program, const char *given) {
    if (given) {
        snprintf(buf, size, "%s", given);
        return;
    }
    const char *base = strrchr(program, '/');
    const char *back = strrchr(program, '\\');
    if (back && (!base || back > base)) base = back;
    base = base ? base + 1 : program;
    snprintf(buf, size, "%s", base);
    char *dot = strrchr(buf, '.');
    if (dot) *dot = '\0';
}

static FILE *open_out(const char *prefix, const char *suffix) {
    char path[512];
    snprintf(path, sizeof path, "%s_%s.out", prefix, suffix);
    FILE *f = fopen(path, "w");
    if (!f) fatal("cannot write '%s'", path);
    return f;
}

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "usage: %s tokens.txt program.txt [output-prefix]\n", argv[0]);
        return 2;
    }

    tokens = st_create();
    symbols = st_create();
    pif_init(&pif);
    load_tokens(argv[1]);

    yyin = fopen(argv[2], "r");
    if (!yyin) fatal("cannot open program '%s'", argv[2]);

    char prefix[512];
    make_prefix(prefix, sizeof prefix, argv[2], argc == 4 ? argv[3] : NULL);
    errors_out = open_out(prefix, "errors");

    yylex();
    fclose(yyin);

    FILE *f = open_out(prefix, "PIF");
    pif_write(&pif, f);
    fclose(f);

    f = open_out(prefix, "ST");
    st_dump(symbols, f);
    fclose(f);

    if (error_count == 0) {
        fprintf(errors_out, "no lexical errors\n");
        printf("%s: lexically correct (%d tokens, %d symbols)\n", argv[2], pif.size, symbols->count);
    } else {
        printf("%s: %d lexical error(s)\n", argv[2], error_count);
    }
    fclose(errors_out);

    pif_free(&pif);
    st_free(symbols);
    st_free(tokens);
    return error_count ? 1 : 0;
}
