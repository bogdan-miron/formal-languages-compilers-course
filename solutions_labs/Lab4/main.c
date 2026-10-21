#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fa.h"
#include "rg.h"

static void usage(void)
{
    fprintf(stderr,
            "usage:\n"
            "  fatool show  file.fa           print the automaton\n"
            "  fatool rg    file.fa           print the regular grammar made from it\n"
            "  fatool check file.fa word      run one word, with the states it goes through\n"
            "  fatool test  file.fa cases.txt run a file of words (+ accept, - reject)\n"
            "  fatool dot   file.fa           print the automaton for graphviz\n");
}

static int cmd_check(const FA *fa, const RG *g, const char *word)
{
    printf("automaton: %s\n", fa->name);
    printf("word: %s\n", word);
    int by_fa = fa_trace(fa, word, stdout);
    int by_rg = rg_accepts(g, word);
    printf("FA: %s\n", by_fa ? "accepted" : "rejected");
    printf("RG: %s\n", by_rg ? "accepted" : "rejected");
    return by_fa == by_rg ? 0 : 1;
}

/* each line is "+ word" or "- word", a lone + or - is the empty word */
static int cmd_test(const FA *fa, const RG *g, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cannot open '%s'\n", path);
        return 2;
    }

    char line[512];
    int total = 0, failed = 0, line_no = 0;
    while (fgets(line, sizeof line, f)) {
        line_no++;
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;
        if ((line[0] != '+' && line[0] != '-') || (line[1] != '\0' && line[1] != ' ')) {
            fprintf(stderr, "%s, line %d: a line starts with + or -, then a space and the word\n", path, line_no);
            fclose(f);
            return 2;
        }

        int expected = line[0] == '+';
        const char *word = line[1] == ' ' ? line + 2 : "";
        int by_fa = fa_accepts(fa, word);
        int by_rg = rg_accepts(g, word);
        total++;

        if (by_fa == expected && by_rg == expected) {
            printf("ok    %-7s %s\n", expected ? "accept" : "reject", word);
        } else {
            failed++;
            printf("FAIL  %-7s %s   (FA %s, RG %s)\n", expected ? "accept" : "reject", word,
                   by_fa ? "accepts" : "rejects", by_rg ? "accepts" : "rejects");
        }
    }
    fclose(f);

    printf("%s: %d words, %d ok, %d failed\n", fa->name, total, total - failed, failed);
    return failed ? 1 : 0;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        usage();
        return 2;
    }
    const char *cmd = argv[1];
    int needs_arg = strcmp(cmd, "check") == 0 || strcmp(cmd, "test") == 0;
    if ((needs_arg && argc != 4) || (!needs_arg && argc != 3) ||
        (!needs_arg && strcmp(cmd, "show") && strcmp(cmd, "rg") && strcmp(cmd, "dot"))) {
        usage();
        return 2;
    }

    static FA fa;
    static RG g;
    if (fa_load(&fa, argv[2])) return 2;
    rg_from_fa(&fa, &g);

    if (strcmp(cmd, "show") == 0) fa_print(&fa, stdout);
    else if (strcmp(cmd, "rg") == 0) rg_print(&g, stdout);
    else if (strcmp(cmd, "dot") == 0) fa_dot(&fa, stdout);
    else if (strcmp(cmd, "check") == 0) return cmd_check(&fa, &g, argv[3]);
    else return cmd_test(&fa, &g, argv[3]);
    return 0;
}
