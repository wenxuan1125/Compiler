%{
#include <stdint.h>
#include <string.h>

#define LIST                strCat(yytext)
#define TOKEN(t)            { LIST; if (opt_tok) printf("<%s>\n", #t); }
#define TOKEN_CHAR(t)       { LIST; if (opt_tok) printf("<%c>\n", (t)); }
#define TOKEN_STRING(t, s)  { LIST; if (opt_tok) printf("<%s: %s>\n", #t, (s)); }
#define TOKEN_STRING_CONSTANT(t, d, s)  { LIST; if (opt_tok) printf("<%s: %.*s>\n", #t, (d), (s)); }

#define MAX_LINE_LENG       512

uint32_t line_num = 1;
char buffer[MAX_LINE_LENG];

static int32_t opt_src = 1;
static int32_t opt_tok = 1;
static char *buffer_ptr = buffer;

static void strCat(const char *Text);

%}

letter      [a-zA-Z]
digit       [0-9]
dec_int     0|[1-9][0-9]* 
floating    {dec_int}"."(0|[0-9]*[1-9])
real        {dec_int}|{floating}
ascii       [\40-\176]
%start comment string

%%
    /*comment: /*  */
<INITIAL>"/*" { LIST; BEGIN comment;}
<comment>"*/" { LIST; BEGIN 0;}
<comment>. { LIST;}

    /*String Constants*/    
<INITIAL>\"/(([\43-\176|\40|\41|\t]|(\"\"))*\") { LIST; BEGIN string; if (opt_tok) printf("<string: ");}
<string>\"\" { LIST; if (opt_tok) printf("\""); }
<string>[\43-\176|\40|\41|\t] { LIST; if (opt_tok) printf("%s", yytext);}
<string>\" { LIST; BEGIN 0; if (opt_tok) printf(">\n");}

    /*
<INITIAL>\"/(([\43-\176|\40|\41]|(\"\"))*\") { LIST; BEGIN string;}
<string>\"\" { strncpy(yytext, yytext, yyleng-1); yymore();}
<string>[\43-\176|\40|\41] { yymore();}
<string>\" { BEGIN 0; TOKEN_STRING_CONSTANT(string, yyleng-1, yytext);}*/
   

    /*Delimiters*/
    
<INITIAL>"," { TOKEN_CHAR(','); }
<INITIAL>";" { TOKEN_CHAR(';'); }
<INITIAL>":" { TOKEN_CHAR(':'); }
<INITIAL>"[" { TOKEN_CHAR('['); }
<INITIAL>"]" { TOKEN_CHAR(']'); }
<INITIAL>")" { TOKEN_CHAR(')'); }
<INITIAL>"(" { TOKEN_CHAR('('); }

    /*Arithmetic, Relational, and Logical Operators*/
<INITIAL>"+" { TOKEN_CHAR('+'); }
<INITIAL>"-" { TOKEN_CHAR('-'); }
<INITIAL>"*" { TOKEN_CHAR('*'); }
<INITIAL>"/" { TOKEN_CHAR('/'); }
<INITIAL>mod { TOKEN(mod); }
<INITIAL>":=" { TOKEN(:=); }
<INITIAL>"<" { TOKEN_CHAR('<'); }
<INITIAL>"<=" { TOKEN(<=); }
<INITIAL>"<>" { TOKEN(<>); }
<INITIAL>">=" { TOKEN(>=); }
<INITIAL>">" { TOKEN_CHAR('>'); }
<INITIAL>"=" { TOKEN_CHAR('='); }
<INITIAL>and { TOKEN(and);} 
<INITIAL>or { TOKEN(or); }
<INITIAL>not { TOKEN(not);}


    /*keywords*/
<INITIAL>array { TOKEN(KWarray);}
<INITIAL>begin { TOKEN(KWbegin);}
<INITIAL>boolean { TOKEN(KWboolean);}
<INITIAL>def { TOKEN(KWdef);}
<INITIAL>do { TOKEN(KWdo);}
<INITIAL>else { TOKEN(KWelse);}
<INITIAL>end { TOKEN(KWend);}
<INITIAL>false { TOKEN(KWfalse);}
<INITIAL>for { TOKEN(KWfor);}
<INITIAL>integer { TOKEN(KWinteger);}
<INITIAL>if { TOKEN(KWif);}
<INITIAL>of { TOKEN(KWof);}
<INITIAL>print { TOKEN(KWprint);}
<INITIAL>read { TOKEN(KWread);}
<INITIAL>real { TOKEN(KWreal);}
<INITIAL>string { TOKEN(KWstring);}
<INITIAL>then { TOKEN(KWthen);}
<INITIAL>to { TOKEN(KWto);}
<INITIAL>true { TOKEN(KWtrue);}
<INITIAL>return { TOKEN(KWreturn);}
<INITIAL>var { TOKEN(KWvar);}
<INITIAL>while { TOKEN(KWwhile);}


    /*Identifiers*/
<INITIAL>[a-zA-Z][a-zA-Z0-9]* { TOKEN_STRING(id, yytext);}
    /*Integer Constants*/
<INITIAL>0[0-7]+ { TOKEN_STRING(oct_integer, yytext);}       /*oct_int*/
<INITIAL>{dec_int} { TOKEN_STRING(integer, yytext);}         /*dec_int*/
    /*Floating-Point Constants*/
<INITIAL>{dec_int}"."(0|[0-9]*[1-9]) { TOKEN_STRING(float, yytext);}
    /*Scientific Notations*/
<INITIAL>{real}[eE](\-?|\+?){dec_int} { TOKEN_STRING(scientific, yytext);}
    

\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, buffer);
    }
    ++line_num;
    buffer[0] = '\0';
    buffer_ptr = buffer;
}

    /*Pseudocomments*/
<INITIAL>"/""/""&"[S]"+"({ascii}|\t)* { LIST; opt_src = 1;}  //&S+
<INITIAL>"/""/""&"[S]"-"({ascii}|\t)* { LIST; opt_src = 0;}  //&S-
<INITIAL>"/""/""&"[T]"+"({ascii}|\t)* { LIST; opt_tok = 1;}  //&T+
<INITIAL>"/""/""&"[T]"-"({ascii}|\t)* { LIST; opt_tok = 0;}  //&T- 


    /*Discarded*/
<INITIAL>[ \t]  { LIST;}  /*Whitespace*/
<INITIAL>"/""/"({ascii}|\t)* { LIST;}    /*commect:// */

    /* Catch the character which is not accepted by rules above */
<INITIAL>. {
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

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./scanner [filename]\n");
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    return 0;
}

