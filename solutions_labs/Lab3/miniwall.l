%option noyywrap yylineno nounput noinput

%{
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

static void emit_token(const char *text);
static void emit_symbol(int code, const char *text);
static void lex_error(const char *fmt, ...);
static void check_address(const char *text);
static void check_comment(const char *text);
static void bad_string(const char *text);
static const char *show(unsigned char c);
%}

D       [0-9]
L       [A-Za-z]
W       ({L}|{D}|_)
STRCH   [A-Za-z0-9 .,:;_/!?(){}\[\]=<>+*%@#&|$^~'-]
IP      {D}+\.{D}+\.{D}+\.{D}+(\/{D}+)?

%%

[ \t\r\n]+                  { /* skip spaces */ }
"#"[^\n\r]*                 { check_comment(yytext); }

\"{STRCH}*\"                { emit_symbol(code_string, yytext); }
\"[^"\n\r]*\"               { bad_string(yytext); }
\"[^"\n\r]*                 { lex_error("unterminated string %s", yytext); }

{IP}                        { check_address(yytext); }
{IP}({L}|_){W}*             { lex_error("invalid address %s", yytext); }

0|[1-9]{D}*                 { emit_symbol(code_number, yytext); }
0{D}+                       { lex_error("number with a leading zero %s", yytext); }
{D}+({L}|_){W}*             { lex_error("invalid identifier %s (starts with a digit)", yytext); }
_{W}*                       { lex_error("invalid identifier %s (starts with an underscore)", yytext); }

allow|and|any|deny|do|dst|else|end|for|from|icmp|if|in|input|not|or|packet|port|print|proto|rule|src|tcp|then|to|udp|within {
                              emit_token(yytext);
                            }
{L}{W}*                     { emit_symbol(code_identifier, yytext); }

\.\.|==|!=|<=|>=            { emit_token(yytext); }
[=<>+*%/.,:()\[\]{}-]       { emit_token(yytext); }

.                           { lex_error("unexpected character %s", show((unsigned char)yytext[0])); }

%%

/* keywords, operators and separators go only in the pif */
static void emit_token(const char *text) {
    Entry *e = st_lookup(tokens, text, NULL);
    if (!e) {
        fprintf(stderr, "internal error: token '%s' is not in the token list\n", text);
        exit(2);
    }
    pif_add(&pif, text, e->value, NULL);
}

/* identifiers and constants go in the st (if new) and in the pif */
static void emit_symbol(int code, const char *text) {
    StPos pos = st_insert(symbols, text, code, NULL);
    pif_add(&pif, text, code, &pos);
}

static void lex_error(const char *fmt, ...) {
    va_list ap;
    char msg[512];
    va_start(ap, fmt);
    vsnprintf(msg, sizeof msg, fmt, ap);
    va_end(ap);
    fprintf(stderr, "Lexical error, line %d: %s\n", yylineno, msg);
    if (errors_out) fprintf(errors_out, "Lexical error, line %d: %s\n", yylineno, msg);
    error_count++;
}

/* turns a character into text for the error message */
static const char *show(unsigned char c) {
    static char buf[8];
    if (isprint(c)) snprintf(buf, sizeof buf, "'%c'", c);
    else snprintf(buf, sizeof buf, "0x%02X", c);
    return buf;
}

/* the scanner reads any digits here, so we check the octets and the prefix:
   1 to 3 digits for an octet, 1 or 2 for the prefix */
static void check_address(const char *text) {
    const char *p = text;
    int ok = 1;
    for (int octet = 0; octet < 4; octet++) {
        int digits = 0;
        while (isdigit((unsigned char)*p)) { p++; digits++; }
        if (digits < 1 || digits > 3) ok = 0;
        if (octet < 3) p++;                 /* skip the dot */
    }
    if (*p == '/') {
        int digits = 0;
        p++;
        while (isdigit((unsigned char)*p)) { p++; digits++; }
        if (digits < 1 || digits > 2) ok = 0;
    }
    if (ok) emit_symbol(code_address, text);
    else lex_error("invalid address %s", text);
}

/* characters allowed in a comment: same as in a string, plus " and tab */
static int comment_char_ok(unsigned char c) {
    return isalnum(c) || strchr(" .,:;_/!?(){}[]=<>+*%@#&|$^~'-\"\t", c) != NULL;
}

static void check_comment(const char *text) {
    for (const char *p = text + 1; *p; p++) {
        if (!comment_char_ok((unsigned char)*p)) {
            lex_error("invalid character %s in comment", show((unsigned char)*p));
            return;
        }
    }
}

/* the quotes are fine but one character is not allowed, find which one */
static void bad_string(const char *text) {
    for (const char *p = text + 1; p[1]; p++) {
        if (!(isalnum((unsigned char)*p) || strchr(" .,:;_/!?(){}[]=<>+*%@#&|$^~'-", *p))) {
            lex_error("invalid character %s in string %s", show((unsigned char)*p), text);
            return;
        }
    }
}
