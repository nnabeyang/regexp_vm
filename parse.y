%{
int yylex(void);
void yyerror(char*);
#include "regexp.h"
static struct Regexp* result;
static int count;
static int nparen;
%}

%union {
  struct Regexp* re;
  int c;
}

%token <c> CHAR EOL
%type <re> alt concat repeat single line
%%
line:alt EOL { 
    result = $1; return 1;
  }
;
alt: concat|
     alt '|' concat {
    $$ = reg(Alt, $1, $3);
    count += 2;// Split + Jmp
  }
;
concat: repeat|
        concat repeat {
    $$ = reg(Cat, $1, $2);
    count += 1;
  }
;
repeat: single
|       single '+' {
    $$ = reg(Plus, $1, NULL);
    count += 1;
  }
|       single '*' {
    $$ = reg(Star, $1, NULL);
    count += 2;
  }
|       single '?' {
    $$ = reg(Quest, $1, NULL);
    count += 1;
  }
;
single: CHAR {
    $$ = reg(Lit, NULL, NULL);
    $$->ch = $1;
    count++;
  }
| '(' alt ')' {
    $$ = reg(Paren, $2, NULL);
    $$->n = ++nparen;
    count += 2;
  }
| '.' {
  $$ = reg(Dot, NULL, NULL);
  count += 1;
}
;
%%

static struct Regexp* result;
static int count;
static int nparen;
static const char* input;
struct Regexp* parse(const char* source) {
  result = NULL;
  count = 0;
  nparen = 0;
  input = source;
  yyparse();
  return result;
}

int yylex(void) {
 int c;
 if(input == NULL || *input == '\0')
   return EOL;
   c = *input++;
   if(strchr("+*|()?.",c))
     return c;
   yylval.c = c;
   return CHAR;
}

void yyerror(char* s) {
  fprintf(stderr, "%s\n", s);
  exit(1);
}

int re_size(void) { return count;}
