%{
// Dummy parser for scanner project.

#include <cassert>

#include "lyutils.h"
#include "astree.h"
#define YYDEBUG 1

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL TOK_DECLID 
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD TOK_NEWSTRING
%token TOK_ORD TOK_CHR TOK_ROOT TOK_FUNCTION TOK_PARAMLIST TOK_PARENTH
%token TOK_PROTOTYPE TOK_RETURNVOID TOK_VARDECL TOK_INDEX

%right TOK_IF TOK_ELSE
%right '='
%left  TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left  '+' '-'
%left  '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_NEW TOK_ORD TOK_CHR
%left  '[' ']' '.' TOK_CALL
%nonassoc TOK_PARENTH

/* start non-terminal symbol for this grammar */
%start start

%%

start      : program                { yyparse_astree = $1; }
          ;
program   : program structdef    { $$ = adopt1 ($1, $2); }
          | program function    { $$ = adopt1 ($1, $2); }
          | program statement    { $$ = adopt1 ($1, $2); }
          | program error '}'    { $$ = $1; }
          | program error ';'   { $$ = $1; }
          |                     { $$ = new_parseroot (); }
          ;


structdef : TOK_STRUCT TOK_IDENT '{'  '}'
            { $2->symbol = TOK_TYPEID; $$ = adopt1 ($1, $2); }
          | TOK_STRUCT TOK_IDENT '{' structfield '}'
              { $2->symbol = TOK_TYPEID; $$ = adopt2 ($1, $2, $4); }
          ;
          
structfield : fielddecl ';'                { $$ = $1; }
              | structfield fielddecl ';'    { $$ = adopt1 ($1, $2); }
              ;
          
fielddecl : basetype TOK_ARRAY TOK_IDENT
              { $3->symbol = TOK_FIELD; $$ = adopt2 ($2, $1, $3); }
          | basetype TOK_IDENT
              { $2->symbol = TOK_FIELD; $$ = adopt1 ($1, $2); }
          ;
          
basetype  : TOK_VOID   { $$ = $1; }
          | TOK_BOOL   { $$ = $1; }
          | TOK_CHAR   { $$ = $1; }
          | TOK_INT    { $$ = $1; }
          | TOK_STRING { $$ = $1; }
          | TOK_IDENT  { $1->symbol = TOK_TYPEID; $$ = $1; }
          ;

function  : identdecl fnparams ')' block
            { $$ = adopt3 
            (new astree(TOK_FUNCTION, 0, 0, 0, ""), $1, $2, $4); }
          | identdecl fnparams ')' ';'
              { $$ = adopt2 
              (new astree (TOK_PROTOTYPE, 0, 0, 0, "") , $1, $2); }
          ;
         
fnparams  : fnparams ',' identdecl    { $$ = adopt1 ($1, $3); }
          | fnparams identdecl        { $$ = adopt1 ($1, $2); }
          | '('                        
              { $1->symbol = TOK_PARAMLIST; $$ = $1; }
          ;
         
identdecl : basetype TOK_ARRAY TOK_IDENT
            { $3->symbol = TOK_DECLID; $$ = adopt2 ($2, $1, $3); }
          | basetype TOK_IDENT
              { $2->symbol = TOK_DECLID; $$ = adopt1 ($1, $2); }
          ;
          
block       : '{' '}'        
            { $1->symbol = TOK_BLOCK; $$ = $1; }
          | '{' blockargs '}'    
              { $1->symbol = TOK_BLOCK; $$ = adopt1 ($1, $2); }
          ;
          
blockargs : blockargs statement    { $$ = adopt1 ($1, $2); }
          | statement            { $$ = $1; }
          ;

statement : block        { $$ = $1; }
          | vardecl        { $$ = $1; }
          | while        { $$ = $1; }
          | ifelse        { $$ = $1; }
          | return        { $$ = $1; }
          | expr ';'    { $$ = $1; }
          | ';'            { $$ = $1; }
          ;

vardecl      : identdecl '=' expr ';'    
            { $2->symbol = TOK_VARDECL; $$ = adopt2 ($2, $1, $3); }
          ;
          
while       : TOK_WHILE '(' expr ')' statement 
            { $$ = adopt2 ($1, $3, $5); }
          ;
          
ifelse      : TOK_IF '(' expr ')' statement
            { $$ = adopt2 ($1, $3, $5); }
          | TOK_IF '(' expr ')' statement TOK_ELSE statement
              { $1->symbol = TOK_IFELSE; 
              $$ = adopt3 ($1, $3, $5, $7); } 
          ;

return    : TOK_RETURN ';'    { $1->symbol = TOK_RETURNVOID; $$ = $1; }
          | TOK_RETURN expr ';' { $$ = adopt1 ($1, $2); }
          ;
          
expr      : expr '=' expr    { $$ = adopt2 ($2, $1, $3); }
          | expr TOK_EQ expr{ $$ = adopt2 ($2, $1, $3); }
          | expr TOK_NE expr{ $$ = adopt2 ($2, $1, $3); }
          | expr TOK_LT expr{ $$ = adopt2 ($2, $1, $3); }
          | expr TOK_LE expr{ $$ = adopt2 ($2, $1, $3); }
          | expr TOK_GT expr{ $$ = adopt2 ($2, $1, $3); }
          | expr TOK_GE expr{ $$ = adopt2 ($2, $1, $3); }
          |    expr '+' expr    { $$ = adopt2 ($2, $1, $3); }
          | expr '-' expr    { $$ = adopt2 ($2, $1, $3); }
          | expr '*' expr     { $$ = adopt2 ($2, $1, $3); }
          | expr '/' expr     { $$ = adopt2 ($2, $1, $3); }
          | expr '%' expr    { $$ = adopt2 ($2, $1, $3); }
          | '+' expr %prec TOK_POS 
              { $1->symbol = TOK_POS; $$ = adopt1 ($1, $2); }
          | '-' expr %prec TOK_NEG
              { $1->symbol = TOK_NEG; $$ = adopt1 ($1, $2); }
          | '!' expr        { $$ = adopt1 ($1, $2); }
          | allocator        { $$ = $1; }
          | call            { $$ = $1; }
          | '(' expr ')'    { $$ = $2; }
          | variable        { $$ = $1; }
          | constant        { $$ = $1; }
          ;
          
allocator : TOK_NEW TOK_IDENT '(' ')'    
            { $2->symbol = TOK_TYPEID; $$ = adopt1 ($1, $2); }
          | TOK_NEW TOK_STRING '(' expr ')'
              { $1->symbol = TOK_NEWSTRING; $$ = adopt1 ($1, $4); }
          | TOK_NEW basetype '[' expr ']'
              { $1->symbol = TOK_NEWARRAY; $2->symbol = TOK_TYPEID; 
              $$ = adopt2 ($1, $2, $4); }
          ;
          
call      : TOK_IDENT '(' ')' 
            { $2->symbol = TOK_CALL; $$ = adopt1 ($2, $1); }
          | TOK_IDENT exprargs ')'
              { $2->symbol = TOK_CALL; $$ = adopt1 ($2, $1); }
          ;

exprargs  : exprargs ',' expr    { $$ = adopt1 ($1, $3); }
          | exprargs expr        { $$ = adopt1 ($1, $2); }
          | '('                    { $1->symbol = TOK_CALL; $$ = $1; }
          ;
          
variable  : TOK_IDENT    { $$ = $1; }
          | expr '[' expr ']'    
              { $2->symbol = TOK_INDEX; $$ = adopt2 ($2, $1, $3); }
          | expr '.' TOK_IDENT    
              { $3->symbol = TOK_FIELD; 
              $$ = adopt2 ($2, $1, $3); }
          ;

constant  : TOK_INTCON        { $$ = $1; }
          | TOK_CHARCON        { $$ = $1; }
          | TOK_STRINGCON    { $$ = $1; }
          | TOK_FALSE        { $$ = $1; }
          | TOK_TRUE        { $$ = $1; }
          | TOK_NULL        { $$ = $1; }
          ;
%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != nullptr);
   return result;
}

