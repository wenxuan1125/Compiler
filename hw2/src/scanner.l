%{
#include <stdint.h>

#include "parser.h"

#define LIST                strCat(yytext)
#define TOKEN(t)            { LIST; if (opt_tok) printf("<%s>\n", #t); }
#define TOKEN_CHAR(t)       { LIST; if (opt_tok) printf("<%c>\n", (t)); }
#define TOKEN_STRING(t, s)  { LIST; if (opt_tok) printf("<%s: %s>\n", #t, (s)); }
#define MAX_LINE_LENG       512

uint32_t line_num = 1;
char buffer[MAX_LINE_LENG];

static int32_t opt_src = 1;
static int32_t opt_tok = 1;
static char string_literal[MAX_LINE_LENG];
static char *buffer_ptr = buffer;

static void strCat(const char *Text);

%}

integer 0|[1-9][0-9]*
float {integer}\.(0|[0-9]*[1-9])

%x CCOMMENT

%%
    /* Delimiter */
"," { TOKEN_CHAR(','); return ',';}
";" { TOKEN_CHAR(';'); return ';';}
":" { TOKEN_CHAR(':'); return ':';}
"(" { TOKEN_CHAR('('); return '(';}
")" { TOKEN_CHAR(')'); return ')';}
"[" { TOKEN_CHAR('['); return '[';}
"]" { TOKEN_CHAR(']'); return ']';}

    /* Operator */
"+"   { TOKEN_CHAR('+'); return '+';}
"-"   { TOKEN_CHAR('-'); return '-';}
"*"   { TOKEN_CHAR('*'); return '*';}
"/"   { TOKEN_CHAR('/'); return '/';}
"mod" { TOKEN(mod); return '%';}
":="  { TOKEN(:=); return ASSIGN;}
"<"   { TOKEN_CHAR('<'); return '<';}
"<="  { TOKEN(<=); return LE;}
"<>"  { TOKEN(<>); return NE;}
">="  { TOKEN(>=); return GE;}
">"   { TOKEN_CHAR('>'); return '>';}
"="   { TOKEN_CHAR('='); return '=';}
"and" { TOKEN(and); return AND;}
"or"  { TOKEN(or); return OR;}
"not" { TOKEN(not); return NOT;}

    /* Keyword */
"array"   { TOKEN(KWarray); return KWARRAY;}
"begin"   { TOKEN(KWbegin); return KWBEGIN;}
"boolean" { TOKEN(KWboolean); return KWBOOLEAN;}
"def"     { TOKEN(KWdef); return KWDEF;}
"do"      { TOKEN(KWdo); return KWDO;}
"else"    { TOKEN(KWelse); return KWELSE;}
"end"     { TOKEN(KWend); return KWEND;}
"false"   { TOKEN(KWfalse); return KWFALSE;}
"for"     { TOKEN(KWfor); return KWFOR;}
"integer" { TOKEN(KWinteger); return KWINTEGER;}
"if"      { TOKEN(KWif); return KWIF;}
"of"      { TOKEN(KWof); return KWOF;}
"print"   { TOKEN(KWprint); return KWPRINT;}
"read"    { TOKEN(KWread); return KWREAD;}
"real"    { TOKEN(KWreal); return KWREAL;}
"string"  { TOKEN(KWstring); return KWSTRING;}
"then"    { TOKEN(KWthen); return KWTHEN;}
"to"      { TOKEN(KWto); return KWTO;}
"true"    { TOKEN(KWtrue); return KWTRUE;} 
"return"  { TOKEN(KWreturn); return KWRETURN;}
"var"     { TOKEN(KWvar); return KWVAR;}
"while"   { TOKEN(KWwhile); return KWWHILE;}

    /* Identifier */
[a-zA-Z][a-zA-Z0-9]* { TOKEN_STRING(id, yytext); return ID;}

    /* Integer (decimal/octal) */
{integer} { TOKEN_STRING(integer, yytext); return DEC;}
0[0-7]+   { TOKEN_STRING(oct_integer, yytext); return OCT;}

    /* Floating-Point */
{float} { TOKEN_STRING(float, yytext); return FLOAT;}

    /* Scientific Notation [Ee][+-]?[0-9]+ */
({integer}|{float})[Ee][+-]?({integer}) { TOKEN_STRING(scientific, yytext); return SCI;}

    /* String */
\"([^"\n]|\"\")*\" {
    char *yyt_ptr = yytext;
    char *str_ptr = string_literal;

    /* Condition guard of the end of string literal
       Check only when *yyt_ptr == '"' */
    while (*(yyt_ptr + 1) != '\0') {
        ++yyt_ptr;

        /* Handle the situation of double quotes */
        if (*yyt_ptr == '"' && *(yyt_ptr + 1) != '\0')
            *str_ptr++ = *yyt_ptr++;

        while (*yyt_ptr != '"')
            *str_ptr++ = *yyt_ptr++;
    }
    *str_ptr = '\0';
    TOKEN_STRING(string, string_literal); return STR;
}
    /* Whitespace */
[ \t]+ { LIST; }

    /* Pseudocomment */
"//&"[ST][+-].* {
    LIST;
    char option = yytext[3];
    switch (option) {
    case 'S':
        opt_src = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'T':
        opt_tok = (yytext[4] == '+') ? 1 : 0;
        break;
    }
}

    /* C++ Style Comment */
"//".* { LIST; }

    /* C Style Comment */
"/*"           { LIST; BEGIN(CCOMMENT); }
<CCOMMENT>"*/" { LIST; BEGIN(INITIAL); }
<CCOMMENT>.    { LIST; }

    /* Newline */
<INITIAL,CCOMMENT>\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, buffer);
    }
    ++line_num;
    buffer[0] = '\0';
    buffer_ptr = buffer;
}

    /* Catch the character which is not accepted by rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}       

%%

static void strCat(const char *Text) {
    while (*Text) {
        *buffer_ptr++ = *Text++;
    }
    *buffer_ptr = '\0';
}
