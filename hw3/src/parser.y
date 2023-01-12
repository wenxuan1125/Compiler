%{
#include "visitor/AstNodeVisitor.hpp"
#include "AST/AstDumper.hpp"
#include "AST/ast.hpp"
#include "AST/program.hpp"
#include "AST/decl.hpp"
#include "AST/variable.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/function.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/print.hpp"
#include "AST/expression.hpp"
#include "AST/BinaryOperator.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/read.hpp"
#include "AST/if.hpp"
#include "AST/while.hpp"
#include "AST/for.hpp"
#include "AST/return.hpp"

#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>

#define YYLTYPE yyltype
#define MAX_ARRAYDECL_LENG 32

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

/* Declared by scanner.l */
extern uint32_t line_num;
extern char buffer[512];
extern FILE *yyin;
extern char *yytext;
/* End */

static AstNode *root;
static AstNodeVisitor *visitor=new AstDumper();

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%code requires {
    class AstNode;
    struct Constant_Value;
    struct Name;
    struct Func_Prototype;
    struct Binary_Op;
    struct Unary_Op;
    #include <vector>
}

    /* For yylval */
%union {
    /* basic semantic value */
    char *identifier;

    AstNode *node;
    //
    int constant_int;
    float constant_real;
    char *constant_str;
    std::vector<AstNode*> *node_list;
    std::vector<char *> *identifier_list;
    Constant_Value *constant;
    Func_Prototype *prototype;
    std::vector<Name*> *name_list;
};

%type <identifier> ProgramName ID FunctionName
%type <node_list> DeclarationList Declarations FunctionList Functions StatementList Statements
%type <node_list> ExpressionList Expressions ArrRefs ArrRefList

%type <node> CompoundStatement Declaration Expression Statement Simple Condition FunctionInvocation
%type <node> FunctionDeclaration FunctionDefinition Function While For Return FunctionCall VariableReference
%type <node> ElseOrNot
    
%type <name_list> IdList
%type <constant> IntegerAndReal LiteralConstant  StringAndBoolean
%type <constant_int> INT_LITERAL NegOrNot
%type <constant_real> REAL_LITERAL 

%type <constant_str> VAR STRING_LITERAL TRUE FALSE ReturnType
%type <constant_str> Type ScalarType ArrType INTEGER REAL STRING BOOLEAN ArrDecl ARRAY

%type <prototype> FormalArg FormalArgs FormalArgList
    
    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
%left OR
%left AND
%right NOT
%left LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN_ /* Use BEGIN_ since BEGIN is a keyword in lex */
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

%%
    /*
       Program Units
                     */

Program:
    ProgramName SEMICOLON
    /* ProgramBody */
    DeclarationList FunctionList CompoundStatement
    /* End of ProgramBody */
    END 
    {
        root = new ProgramNode(@1.first_line, @1.first_column,
                               $1, "void", $3, $4, $5);

        free($1);
    }
;

ProgramName:
    ID
;

DeclarationList:    // node_list
    Epsilon 
    { 
        $$=NULL;
    }
    |
    Declarations 
    {
        $$=$1;
    }
;

Declarations:   
    Declaration 
    {   
        std::vector<AstNode*> *v=new std::vector<AstNode*>;
        v->push_back( $1);
        $$=v;

    }
    |
    Declarations Declaration
    {
        $$=$1;
        $$->push_back($2);

    }
;

FunctionList:
    Epsilon
    {
        $$=NULL;
    }
    |
    Functions
    {
        $$=$1;
    }
;

Functions:
    Function
    {
        std::vector<AstNode*> *v=new std::vector<AstNode*>;
        v->push_back( $1);
        $$=v;
    }
    |
    Functions Function
    {   
        $$=$1;
        $$->push_back($2);
        
    }
;

Function:
    FunctionDeclaration
    {
        $$=$1;
    }
    |
    FunctionDefinition
    {
        $$=$1;
    }
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON
    {
         if( $3==NULL)
            $$ = new FunctionNode(@1.first_line, @1.first_column,$1, $5, NULL, NULL, NULL); 
        else
            $$ = new FunctionNode(@1.first_line, @1.first_column,$1, $5, $3->decl_list, $3->type_list, NULL);
    }
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType
    CompoundStatement
    END
    {
        if( $3==NULL)
            $$ = new FunctionNode(@1.first_line, @1.first_column,$1, $5, NULL, NULL, $6); 
        else
            $$ = new FunctionNode(@1.first_line, @1.first_column,$1, $5, $3->decl_list, $3->type_list, $6);
    }
;

FunctionName:
    ID
;

FormalArgList:
    Epsilon
    {
        $$=NULL;
    }
    |
    FormalArgs
    {
        $$=$1;
    }
;

FormalArgs: 
    FormalArg
    {
        Func_Prototype *f=new Func_Prototype;
        std::vector<char*> *type_list=new std::vector<char*>;

        for( int i=0; i<$1->type_list->size(); i++){
            char* c = strdup($1->type_list->at(i));
            type_list->push_back(c);
        }
             
        f->decl_list=$1->decl_list;
        f->type_list=type_list;
        $$=f;
    }
    |
    FormalArgs SEMICOLON FormalArg
    {
        
        $$=$1;
        for( int i=0; i< $3->type_list->size(); i++){
            char* c = strdup($3->type_list->at(i));
            $$->type_list->push_back(c);
        }
        $$->decl_list->push_back($3->decl_list->at(0));

    }
;

FormalArg:  // prototype
    IdList COLON Type
    {
        std::vector<AstNode*> *var_list=new std::vector<AstNode*>;
        std::vector<char*> *type_list=new std::vector<char*>;
        for( int i=0; i<$1->size(); i++){

            VariableNode *v=new VariableNode($1->at(i)->line, $1->at(i)->col, $1->at(i)->id, $3, NULL);

            char s[MAX_ARRAYDECL_LENG+1];
            strcpy( s, $3);

            var_list->push_back(v);
            type_list->push_back(s);
        }
        AstNode *n = new DeclNode(@1.first_line, @1.first_column, var_list); 
        Func_Prototype *f=new Func_Prototype;
        std::vector<AstNode*> *d= new std::vector<AstNode*>;
        f->decl_list=d;
        f->decl_list->push_back(n);
        f->type_list=type_list;
        $$=f;
    }
;

IdList:     // id_list
    ID
    {
        $$=new std::vector<Name*>;
        Name *n=new Name;
        n->id=$1;
        n->line=@1.first_line;
        n->col=@1.first_column;
        $$->push_back(n);
    }
    |
    IdList COMMA ID
    {
        $$=$1;
        Name *n=new Name;
        n->id=$3;
        n->line=@3.first_line;
        n->col=@3.first_column;
        $$->push_back(n);
    }
;

ReturnType:
    COLON ScalarType
    {
        $$=$2;
    }
    |
    Epsilon
    {
        char s[MAX_ARRAYDECL_LENG+1];
        strcpy( s, "void");
        $$=s;
    }
;

    /*
       Data Types and Declarations
                                   */

Declaration:    // Astnode
    VAR IdList COLON Type SEMICOLON 
    {
        std::vector<AstNode*> *var_list=new std::vector<AstNode*>;
        char s[MAX_ARRAYDECL_LENG+1];
        for( int i=0; i<$2->size(); i++){

            VariableNode *v=new VariableNode($2->at(i)->line, $2->at(i)->col, $2->at(i)->id, $4, NULL);
            var_list->push_back(v);
        }
        $$ = new DeclNode(@1.first_line, @1.first_column, var_list);
        
    }
    |
    VAR IdList COLON LiteralConstant SEMICOLON
    {
        ConstantValueNode *c=new ConstantValueNode($4->line, $4->col, *($4));
        std::vector<AstNode*> *var_list=new std::vector<AstNode*>;
        char *s;
        for( int i=0; i<$2->size(); i++){
            if( $4->int_type){
                s="integer"; 
            }
            else if( $4->real_type){
                s="real"; 
            }
            else if( $4->str_type){
                s="string"; 
            }
            else if( $4->bool_type){
                s="boolean";
            }

            VariableNode *v=new VariableNode($2->at(i)->line, $2->at(i)->col, $2->at(i)->id, s, c);
            var_list->push_back(v);
        }
          
        $$ = new DeclNode(@1.first_line, @1.first_column, var_list);
    }
;

Type:
    ScalarType
    {
        
        $$=$1;
        
    }
    |
    ArrType
    {
        $$=$1;
    }
    
;

ScalarType:
    INTEGER
    {
        $$="integer";
        
    }
    |
    REAL
    {
        $$="real";
    }
    |
    STRING
    {
        char s[MAX_ARRAYDECL_LENG+1];
        strcpy(s, "string");
        $$=s;
    }
    |
    BOOLEAN
    {
        $$="boolean";
    }
;

ArrType:
    ArrDecl ScalarType
    {
        char s1[MAX_ARRAYDECL_LENG+1];
        char s2[MAX_ARRAYDECL_LENG+1];
        char s3[MAX_ARRAYDECL_LENG+1];
        strcpy(s1, $2);
        strcpy(s3, $1);


        strcpy(s2, " ");
        strcat( s1, s2);
        strcat( s1, s3);
        
        $$=s1;
    }
;

ArrDecl:
    ARRAY INT_LITERAL OF
    {
        char s1[MAX_ARRAYDECL_LENG+1];
        char s2[MAX_ARRAYDECL_LENG+1];
        strcpy(s1, "[");
        sprintf(s2,"%d", $2);
        strcat( s1, s2);
        strcpy(s2, "]");
        strcat( s1, s2);
        
        $$=s1;
    }
    |
    ArrDecl ARRAY INT_LITERAL OF
    {
        char s1[MAX_ARRAYDECL_LENG+1];
        char s2[MAX_ARRAYDECL_LENG+1];
        strcpy(s1, $1);
        strcpy(s2, "[");
        strcat( s1, s2);
        sprintf(s2,"%d", $3);
        strcat( s1, s2);
        strcpy(s2, "]");
        strcat( s1, s2);

        $$=s1;
    } 
;

LiteralConstant:
    NegOrNot INT_LITERAL
    {   
        $$=new Constant_Value;
        $$->int_value=$1*$2;
        $$->int_type=true;
        if( $1==1){
            $$->col= @2.first_column;
            $$->line=@2.first_line;
        }
        else{
            $$->col= @1.first_column;
            $$->line=@1.first_line;
        }
            
    }
    |
    NegOrNot REAL_LITERAL
    {   
        $$=new Constant_Value;
        $$->real_value=$1*$2;
        $$->real_type=true;
        if( $1==1){
            $$->col= @2.first_column;
            $$->line=@2.first_line;
        }
        else{
            $$->col= @1.first_column;
            $$->line=@1.first_line;
        }
    }
    |
    StringAndBoolean
    {
        $$=$1;
        $$->col= @1.first_column;
        $$->line=@1.first_line;
    }
;

NegOrNot:
    Epsilon
    {
        $$=1;
    }
    |
    MINUS %prec UNARY_MINUS
    {
        $$=-1;
    }
;

StringAndBoolean:
    STRING_LITERAL
    {
        $$=new Constant_Value;
        $$->str_value=$1;
        $$->str_type=true;
    }
    |
    TRUE
    {
        $$=new Constant_Value;
        $$->str_value="true";
        $$->bool_type=true;
    }
    |
    FALSE
    {
        $$=new Constant_Value;
        $$->str_value="false";
        $$->bool_type=true;
    }
;

IntegerAndReal:
    INT_LITERAL
    {
        $$=new Constant_Value;
        $$->int_value=$1;
        $$->int_type=true;

    }
    |
    REAL_LITERAL
    {
        $$=new Constant_Value;
        $$->real_value=$1;
        $$->real_type=true;

    }
;

    /*
       Statements
                  */

Statement:
    CompoundStatement
    {
        $$=$1;
    }
    |
    Simple
    {
        $$=$1;
    }
    |
    Condition
    {
        $$=$1;
    }
    |
    While
    {
        $$=$1;
    }
    |
    For
    {
        $$=$1;
        
    }
    |
    Return
    {
        $$=$1;
    }
    |
    FunctionCall
    {
        $$=$1;
    }
;

CompoundStatement:
    BEGIN_
    DeclarationList
    StatementList
    END
    {
        $$ = new CompoundStatementNode(@1.first_line, @1.first_column, $2, $3);
    }
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON
    {
        $$ = new AssignmentNode(@2.first_line, @2.first_column, $1, $3);
    }
    |
    PRINT Expression SEMICOLON
    {
        $$ = new PrintNode(@1.first_line, @1.first_column, $2);
    }
    |
    READ VariableReference SEMICOLON
    {
        $$ = new ReadNode(@1.first_line, @1.first_column, $2);
    }
;

VariableReference:
    ID ArrRefList
    {
        $$= new VariableReferenceNode(@1.first_line, @1.first_column, $1, $2);
    }
;

ArrRefList:
    Epsilon
    {
        $$=NULL;
    }
    |
    ArrRefs
    {
        $$=$1;
    }
;

ArrRefs:
    L_BRACKET Expression R_BRACKET
    {
        std::vector<AstNode*> *a=new std::vector<AstNode*>;
        a->push_back( $2);
        $$=a;
    }
    |
    ArrRefs L_BRACKET Expression R_BRACKET
    {
        $$=$1;
        $$->push_back($3);
    }
;

Condition:
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF
    {
        if($4==NULL){
            $$= new IfNode(@1.first_line, @1.first_column, $2, $4, NULL);
        }
        else{
            $$= new IfNode(@1.first_line, @1.first_column, $2, $4, $5);
        }
    }
;

ElseOrNot:
    ELSE
    CompoundStatement
    {
        $$=$2;
    }
    |
    Epsilon
    {
        $$=NULL;
    }
;

While:
    WHILE Expression DO
    CompoundStatement
    END DO
    {
        $$= new WhileNode(@1.first_line, @1.first_column, $2, $4);
    }
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO
    {
        // loop_variable
        std::vector<AstNode*> *var_list=new std::vector<AstNode*>;
        VariableNode *v = new VariableNode(@2.first_line, @2.first_column, $2, "integer", NULL);
        var_list->push_back(v);
        DeclNode *d = new DeclNode(@2.first_line, @2.first_column, var_list);

        // initial
        VariableReferenceNode *vr = new VariableReferenceNode(@2.first_line, @2.first_column, $2, NULL);
        Constant_Value *cv1 =new Constant_Value;
        cv1->int_value=$4;
        cv1->int_type=true;
        ConstantValueNode *c1 = new ConstantValueNode(@4.first_line, @4.first_column, *(cv1));
        AssignmentNode *a = new AssignmentNode(@3.first_line, @3.first_column, vr, c1);

        // condition
        Constant_Value *cv2 =new Constant_Value;
        cv2->int_value=$6;
        cv2->int_type=true;
        ConstantValueNode *c2 = new ConstantValueNode(@6.first_line, @6.first_column, *(cv2));
          
        $$ = new ForNode(@1.first_line, @1.first_column, d, a, c2, $8);
    }
;

Return:
    RETURN Expression SEMICOLON
    {
        $$=new ReturnNode( @1.first_line, @1.first_column, $2);
    }
;

FunctionCall:
    FunctionInvocation SEMICOLON
    {
        $$=$1;
    }
;

FunctionInvocation:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS
    {
        $$=new FunctionInvocationNode( @1.first_line, @1.first_column, $1, $3);
    }
;

ExpressionList:
    Epsilon
    {
        $$=NULL;
    }
    |
    Expressions
    {
        $$=$1;
    }
;

Expressions:
    Expression
    {
        std::vector<AstNode*> *e=new std::vector<AstNode*>;
        e->push_back( $1);
        $$=e;
        
    }
    |
    Expressions COMMA Expression
    {
        $$=$1;
        $$->push_back($3);
    }
;

StatementList:
    Epsilon
    {
        $$=NULL;
    }
    |
    Statements
    {
        $$=$1;
    }
;

Statements:
    Statement
    {
        std::vector<AstNode*> *v=new std::vector<AstNode*>;
        v->push_back( $1);
        $$=v;
    }
    |
    Statements Statement
    {
        $$=$1;
        $$->push_back($2);
    }
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS
    {
        $$=$2;
    }
    |
    MINUS Expression %prec UNARY_MINUS
    {
        Unary_Op u;
        u.neg=true;

        $$=new UnaryOperatorNode( @1.first_line, @1.first_column, u, $2);
    }
    |
    Expression MULTIPLY Expression
    {
        Binary_Op b;
        b.mul=true;

        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression DIVIDE Expression
    {
        Binary_Op b;
        b.div=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression MOD Expression
    {
        Binary_Op b;
        b.mod=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression PLUS Expression
    {
        Binary_Op b;
        b.add=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression MINUS Expression
    {
        Binary_Op b;
        b.sub=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression LESS Expression
    {
        Binary_Op b;
        b.lt=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression LESS_OR_EQUAL Expression
    {
        Binary_Op b;
        b.le=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression GREATER Expression
    {
        Binary_Op b;
        b.gt=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression GREATER_OR_EQUAL Expression
    {
        Binary_Op b;
        b.ge=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression EQUAL Expression
    {
        Binary_Op b;
        b.eq=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression NOT_EQUAL Expression
    {
        Binary_Op b;
        b.ne=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    NOT Expression
    {
        Unary_Op u;
        u.NOT=true;

        $$=new UnaryOperatorNode( @1.first_line, @1.first_column, u, $2);
    }
    |
    Expression AND Expression
    {
        Binary_Op b;
        b.AND=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    Expression OR Expression
    {
        Binary_Op b;
        b.OR=true;
        $$=new BinaryOperatorNode( @2.first_line, @2.first_column, b, $1, $3);
    }
    |
    IntegerAndReal
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column,*($1));
    }
    |
    StringAndBoolean
    {
        $$ = new ConstantValueNode(@1.first_line, @1.first_column,*($1));
    }
    |
    VariableReference
    {
        $$=$1;
    }
    |
    FunctionInvocation
    {
        $$=$1;
    }
;

    /*
       misc
            */
Epsilon:
;
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
    if (argc < 2) {
        fprintf(stderr, "Usage: ./parser <filename> [--dump-ast]\n");
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    assert(yyin != NULL && "fopen() fails.");

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        //root->print();
        root->accept(*visitor);
    }
    //root->print();
    //root->accept(visitor);
    
    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}
