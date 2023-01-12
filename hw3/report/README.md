# hw3 report

|||
|-:|:-|
|Name|成文瑄|
|ID|0716004|

## How much time did you spend on this project

e.g. 2 hours.

一個星期(如果加上把spec看懂的話可能要多個3天)。

## Project overview

Describe the project structure and how you implemented it.

首先是scanner的部分，在return token前，我把一些在parser中terminal需要的attribite資訊利用yylval.terminal_type傳給parser（這裡的type要和parser中union的type對起來），這樣parser中才可以透過$拿到terminal的attribute資訊。

    /* Integer (decimal/octal) */
{integer} {
    TOKEN_STRING(integer, yytext);
    yylval.constant_int=atoi(yytext);
    return INT_LITERAL;
}

之後在parser中要宣告nonterminal的attribute的type（就是利用$拿到的東西的type，也是宣告在union中，然後這裡面如果是c沒有但c++有的東西要宣告成指標，像是vector，然後和我自己寫的struct等等要在code requires中宣告和include才能用)

%type <identifier> ProgramName ID FunctionName
%type <node_list> DeclarationList Declarations FunctionList Functions StatementList Statements
%type <node_list> ExpressionList Expressions ArrRefs ArrRefList

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

%code requires {
    class AstNode;
    struct Constant_Value;
    struct Name;
    struct Func_Prototype;
    struct Binary_Op;
    struct Unary_Op;
    #include <vector>
}

再來是在cpp和hpp寫node需要的內容，然後在parser中利用$得到nonterminal和terminal的attribute，然後建立node，把各種node組成ast。
以declaration node為例子說明：
首先因為declaration node需要variable的list，所以我在它的class中宣告了一個verctor<AstNode*> *的variable_list指標，然後把它的constructor補齊（就是把傳進來的的東西賦值給我的東西，其他node的class也是像這樣寫）。

in .hpp
  std::vector<AstNode*> *variable_list;  // list of variable nodes
  
in .cpp
 DeclNode::DeclNode(const uint32_t line, const uint32_t col,
                    std::vector<AstNode*> *p_var_list)
    : AstNode{line, col}, variable_list(p_var_list) {}

把class寫完之後，回到parser中把node建起來。
parser中有兩種declaration node的建立方式：

第一種：
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
    
這裡要建的declaration node，因為參數需要variable的list，但這條的nonterminal中沒有variable node（idlist的type是我寫的struct的vector的指標，我的裡面存了id和id的line和column，因為在variable node中的location要用的是id的位置，所以不能直接在這裡用@2放idlist的位置，而是在它還是id 還沒reduce成idlist之前就要把它存起來），所以用for迴圈去把idlist中的id建成variable node再把它push到variable list中，最後建declaration node再把它傳進去。

第二種：
Declaration:    // Astnode
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

這種declaration node下面的variable node之下還有constantvalue node（上一種沒有），所以在這裡建variable node之前還要先建constantvalue node，接著建variable node時，因為我是把literalconstant包成一個struct，裡面存了它是那種type和它的value，所以需要用if來判斷，再把它放到variable node的參數中。

像這樣子建好declration node之後，他要串成declaration list，最後傳給program node。
串成declaration list也是先new一個vector<AstNode*>的指標，然後一個一個把東西push進去。
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
    
最後在program這裡就可以拿到declaration list，然後用它來建立program node。
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

其他的node和建ast也是相同的建立方式，只是每個node傳的參數不太一樣。

實現visitor pattern：
因為我是先寫普通的多型，寫完之後才改成visitor pattern的，所以我的code中有多型用的print和visitor pattern 用的accept、visitchildnodes和get...。
每個node中的accept是用來接受visitor的拜訪的(裡面是visit這顆node)，因為visitor這個function是參數不一樣的多載，但是在parser中宣告的type都是AstNode*，如果直接在parser中visit那顆node，它只知道它是AstNode*，不知道他是哪一種node，它就沒辦法呼叫對應的visit function。所以透過accept這個function，因為它裡面會在visit中放入this*，這樣它就可以知道它是哪種node，也就能呼叫對應的visit function。然後在visit這個function 中，就會印出要印的東西，但因為對node來說，visitor是外人，沒辦法直接存取private的變數們，所以在node需要一些get function來給外面的人變數值。印完自己的東西後就呼叫visitchildnodes去拜訪小孩，裡面就是去呼叫小孩的accept。就這樣一直下去就可以印完所有的東西。

void BinaryOperatorNode::accept(AstNodeVisitor &p_visitor){
    p_visitor.visit(*this);
}

void BinaryOperatorNode::visitChildNodes(AstNodeVisitor &p_visitor) {
    if( left!= NULL)
        left->accept(p_visitor);
    if( right!= NULL)
        right->accept(p_visitor);
}

const char *ProgramNode::getNameCString() const { return name.c_str(); }

## What is the hardest you think in this project

Let us know, if any.

我覺得最難的部分是會不知道要怎麼開始，spec雖然寫的很多，但可能太多了而且又是第一次寫這種東西，一開始看完spec真的是很混亂。

還有就是字串的處理弄了很久，我在做的時候常常碰到在底下的字串印出來是對的，但等到reduce到上層時字串就會莫名其妙的被覆蓋掉（有確認過我沒有自己去寫到它）。後來上網查到發現好像是yytext這個指標的問題，因為它可能會一直去接下一個token的match的字串，所以可能不知道哪時候前面的東西就會自己消失了，所以若是直接賦值給yylval，parser又直接拿$去取的話，它可能那時候就不知道去哪了，解法是在scanner給yylval前可以用strdup給他一個memory的位置！

## Feedback to T.A.s

> Please help us improve our assignment, thanks.

我覺得這次的spec檔案實在是太多太分散了，一開始點進去的那個spec雖然寫了很多實作的細節和方式，但其他也很重要的spec像是寫那個每個node裡需要有哪些東西的spec或是visitor pattern的spec被放在一開始那個spec的小小文字的連結，很容易就會讓別人忽略它的存在。另外，雖然spec寫的很完整，但還是會希望如果可以的話，老師可以在上課的時候，稍微解釋一下整個作業的邏輯，在寫作業時花在看spec和理解自己到底要做什麼的時間應該會就少一點，寫起作業應該也會快樂一點（嗎？）

