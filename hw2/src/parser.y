%{
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern int32_t line_num;   /* declared in scanner.l */
extern char buffer[512];  /* declared in scanner.l */
extern FILE *yyin;        /* declared by lex */
extern char *yytext;      /* declared by lex */
    
extern int yylex(void); 
static void yyerror(const char *msg);
%}
%left AND OR NOT
%left '<' LE NE GE '>' '='
%left '-'
%left '+'
%left '/' '%'
%left '*' 
%token AND OR NOT ASSIGN GE LE NE 
%token KWARRAY KWBEGIN KWBOOLEAN KWDEF KWDO KWELSE KWEND KWFALSE KWFOR KWINTEGER KWIF KWOF 
%token KWPRINT KWREAD KWREAL KWSTRING KWTHEN KWTO KWTRUE KWRETURN KWVAR KWWHILE
%token ID DEC OCT FLOAT SCI STR


%% 
    /*program_name: ID;*/

program: ID ';' decl_list func_list compound KWEND;


    /* function */
arg_list: | non_empty_arg_list;
non_empty_arg_list: arg_decl | non_empty_arg_list ';' arg_decl;
id_list: | non_empty_id_list;
non_empty_id_list: ID | non_empty_id_list ',' ID;
arg_decl: non_empty_id_list ':' scalar_type | non_empty_id_list ':' KWARRAY integer_const;

func_decl: ID '(' arg_list ')' ':' scalar_type ';'
            | ID '(' arg_list ')' ';';
func_kwdef: ID '(' arg_list ')' ':' scalar_type compound KWEND
            | ID '(' arg_list ')' compound KWEND;
func_list: | func_list func_decl | func_list func_kwdef;

    /* data types and declaration */
scalar_type: KWINTEGER | KWREAL | KWSTRING | KWBOOLEAN;

    /* for multi-dimension array */
type: type1 | scalar_type | KWARRAY integer_const;
type1: type1 KWOF type2 | KWARRAY integer_const KWOF type2;
type2: KWARRAY integer_const | scalar_type;

scalar_decl: KWVAR non_empty_id_list ':' scalar_type ';';
struct_decl: KWVAR non_empty_id_list ':' KWARRAY integer_const KWOF type ';';
integer_const: DEC | OCT;
const_decl: KWVAR non_empty_id_list ':' literal_const ';';
decl_list: | decl_list const_decl | decl_list scalar_decl | decl_list struct_decl;
literal_const: integer_const | FLOAT | SCI | STR | KWTRUE | KWFALSE;

    /* statement */
stat: compound | simple | condition | while | for | return | procedure;
stat_list: | stat_list stat;
compound: KWBEGIN decl_list stat_list KWEND;

simple: var_ref ASSIGN expr ';'
    /*| KWPRINT var_ref ';'*/
        | KWPRINT expr ';'
        | KWREAD var_ref ';';

var_ref: ID | arr_ref;
arr_ref: arr_ref '[' expr ']' | ID '[' expr ']';

condition: KWIF expr KWTHEN compound KWELSE compound KWEND KWIF | KWIF expr KWTHEN compound KWEND KWIF;
while: KWWHILE expr KWDO compound KWEND KWDO;
for: KWFOR ID ASSIGN integer_const KWTO integer_const KWDO compound KWEND KWDO;
return: KWRETURN expr ';';
procedure: func_invo ';';
func_invo:  ID '(' expr_list ')';

    /* expression */
expr: '-' expr %prec '*'
    | expr '*' expr
    | expr '/' expr
    | expr '%' expr
    | expr '+' expr
    | expr '-' expr
    | expr '<' expr
    | expr '>' expr
    | expr '=' expr
    | expr GE expr
    | expr LE expr
    | expr NE expr
    | expr AND expr
    | expr OR expr
    | NOT expr
    | '(' expr ')'
    | literal_const | var_ref | func_invo;
 
expr_list: | non_empty_expr_list;
non_empty_expr_list: expr | non_empty_expr_list ',' expr;

    

%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, buffer, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./parser <filename>\n");
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    assert(yyin != NULL && "fopen() fails.");

    yyparse();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}

