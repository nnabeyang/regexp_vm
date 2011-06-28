#include "regexp.h"
#include <string.h>
struct Inst {
  int opcode;
  int c;
  struct Inst* x;
  struct Inst* y;
  int gen;
};
// opcode
enum {
  Char = 1,
  Match,
};
struct Prog {
  struct Inst* start;
  int len;
};
static struct Inst* pc;
void emit(struct Regexp* re);
struct Prog* compile(struct Regexp* re) {
  struct Prog* p;
  p = malloc(sizeof(*p) + re_size() * sizeof(p->start[0]));
  p->start = (struct Inst*)(p+1);
  pc = p->start;
  emit(re);
  pc->opcode = Match;
  pc++;
  p->len = pc - p->start;
  return p;
}
void emit(struct Regexp* re) {
  switch(re->type) {
    default:
      assert(0);
    case Lit:
      pc->opcode = Char;
      pc->c = re->ch;
      pc++;
      break;
    case Cat:
      emit(re->left);
      emit(re->right);
  }
}
void prog_to_str(char* str, struct Prog* p) {
  struct Inst *pc, *end;
  end = p->start + p->len;
  for(pc = p->start; pc < end; pc++) {
    switch(pc->opcode) {
      default:
        assert(0);
      case Char:
        sprintf(str, "%d. char %c\n", (int)(pc-p->start), pc->c);
	str = str + strlen(str);
	break;
      case Match:
        sprintf(str, "%d. match\n", (int)(pc-p->start));
	str = str + strlen(str);
	break;
    }
  }
}
int is_match(struct Prog* prog,const char* input) {
  const char* sp;
  struct Inst* pc = prog->start;
  int matched = 0;
  for(sp = input; sp != '\0'; sp++) {
    switch(pc->opcode) {
    case Char:
      if(*sp != pc->c)
        goto BreakFor;
      pc++;
      break;
    case Match:
      matched = 1;
      goto BreakFor;
    }
  }
  BreakFor:
            return matched;
}
struct Regexp* reg(int type, struct Regexp* left, struct Regexp* right) {
struct Regexp* re = (struct Regexp*)malloc(sizeof(struct Regexp));
  re->type = type; re->left = left; re->right = right;
  return re;
}
void reg_to_str(char* str, struct Regexp* re) {
  switch(re->type) {
    default:
      assert(0);
    case Lit:
      sprintf(str, "Lit(%c)", re->ch);
      break;
    case Cat: {
      char lbuf[80], rbuf[80];
      reg_to_str(lbuf, re->left);
      reg_to_str(rbuf, re->right);
      sprintf(str, "Cat(%s, %s)", lbuf, rbuf);
      break;
      }
  }
}
void test(void);
int main(int argc, char* argv[]) {
  if(argc == 2 && !strcmp(argv[1], "test")) {
    test();
    return 0;
  }
  if(argc < 3) {
    fprintf(stderr, "usage:%s regexp string...\n", argv[0]);
    return 1;
  }
  struct Regexp* re = parse(argv[1]);
  struct Prog* prog = compile(re);
  int i;
  for(i = 2; i < argc; i++) {
    if(is_match(prog, argv[i]))
      printf("%s\n", argv[i]);
  }
  return 0;
}
void test_reg(void) {
struct Regexp* lit1 = reg(Lit, NULL, NULL);
lit1->ch = 'a';
struct Regexp* lit2 = reg(Lit, NULL, NULL);
lit2->ch = 'b';

char str[80];
reg_to_str(str, lit1);
assert(!strcmp("Lit(a)", str));

struct Regexp* cat = reg(Cat, lit1, lit2);

reg_to_str(str, cat);
assert(!strcmp("Cat(Lit(a), Lit(b))", str));

free(lit1); free(lit2); free(cat);
}

void test_parse_concat(void) {
struct Regexp* re1 = parse("a");
  assert(1 == re_size());
char str[80];
reg_to_str(str, re1);
assert(!strcmp("Lit(a)", str));
struct Regexp* re2 = parse("ab");
  assert(4 == re_size());
reg_to_str(str, re2);
assert(!strcmp("Cat(Lit(a), Lit(b))", str));
}

void test_compile(void) {
  struct Regexp* re = parse("ab");
  assert(4 == re_size());
  struct Prog* prog = compile(re);
  char str[80];
  prog_to_str(str, prog);
  char expect[] =
  "0. char a\n"
  "1. char b\n"
  "2. match\n"
  ;
  assert(!strcmp(expect, str));
}

void test_is_match(void) {
  char input[] = "ab";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  assert(is_match(prog, input));
  assert(!is_match(prog, "bc"));

}

void test(void) {
  test_reg();
  test_parse_concat();
  test_compile();
  test_is_match();
}
