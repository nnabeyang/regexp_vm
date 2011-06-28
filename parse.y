%{
int yylex(void);
void yyerror(char*);
#include "regexp.h"
static struct Regexp* result;
static int count;
%}

%union {
  struct Regexp* re;
  int c;
}

%token <c> CHAR EOL
%type <re> concat repeat single line

%%
line:concat EOL { 
    result = $1; return 1;
  }
;
concat: repeat|
        concat repeat {
    $$ = reg(Cat, $1, $2);
    count += 2;
  }
repeat: single|
        single '+' {
    $$ = reg(Plus, $1, NULL);
    count += 2;
  }
single: CHAR {
    $$ = reg(Lit, NULL, NULL);
    $$->ch = $1;
    count++;
  }
;
%%

static struct Regexp* result;
static int count;
static const char* input;
struct Regexp* parse(const char* source) {
  result = NULL;
  count = 0;
  input = source;
  yyparse();
  return result;
}

int yylex(void) {
 int c;
 if(input == NULL || *input == '\0')
   return EOL;
   c = *input++;
   if(c == '+')
     return c;
   yylval.c = c;
   return CHAR;
}

void yyerror(char* s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int re_size(void) { return count;}
