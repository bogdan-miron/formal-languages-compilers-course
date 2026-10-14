#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "symtab.h"
#include "pif.h"

/* things shared between main.c and the scanner (miniwall.l) */

extern SymTable *symbols;   /* identifiers and constants */
extern SymTable *tokens;    /* the token list from tokens.txt */
extern Pif pif;             /* program internal form */
extern FILE *errors_out;    /* errors are also written here */
extern int error_count;

/* codes of the 4 classes, read from tokens.txt */
extern int code_identifier, code_number, code_string, code_address;

int yylex(void);
extern FILE *yyin;

#endif
