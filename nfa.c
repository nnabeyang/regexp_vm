#include "regexp.h"
#include <string.h>
struct Inst {
  int opcode;
  int c;
  int n;// nparen
  struct Inst* x;
  struct Inst* y;
  int gen;
};
// opcode
enum {
  Char = 1,
  Split,
  Jmp,
  Save,
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
  p = malloc(sizeof(*p) + (re_size()+1) * sizeof(p->start[0]));
  p->start = (struct Inst*)(p+1);
  pc = p->start;
  emit(re);
  pc->opcode = Match;
  pc++;
  p->len = pc - p->start;
  return p;
}
void emit(struct Regexp* re) {
  struct Inst *p1, *p2;
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
      break;
    case Plus:
      p1 = pc;
      emit(re->left);
      pc->opcode = Split;
      pc->x = p1;
      p2 = pc;
      pc++;
      p2->y = pc;
      break;
    case Star:
      pc->opcode = Split;
      p1 = pc++;
      p1->x = pc;
      emit(re->left);
      pc->opcode = Jmp;
      pc->x = p1;
      pc++;
      p1->y = pc;
      break;
    case Alt:
      pc->opcode = Split;
      p1 = pc++;
      p1->x = pc;
      emit(re->left);
      pc->opcode = Jmp;
      p2 = pc++;
      p1->y = pc;
      emit(re->right);
      p2->x = pc;
      break;
    case Paren:
      pc->opcode = Save;
      pc->n = 2*re->n;
      pc++;
      emit(re->left);
      pc->opcode = Save;
      pc->n = 2*re->n +1;
      pc++;
      break;
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
      case Split:
        sprintf(str, "%d. split %d, %d\n",
	(int)(pc-p->start),(int)(pc->x-p->start), (int)(pc->y-p->start));
	str = str + strlen(str);
	break;
      case Jmp:
        sprintf(str, "%d. jmp %d\n",
	(int)(pc-p->start),(int)(pc->x-p->start));
	str = str + strlen(str);
	break;
      case Save:
        sprintf(str, "%d. save [%d]\n",
	(int)(pc-p->start),(int)(pc->n));
	str = str + strlen(str);
	break;
    }
  }
}
struct Thread {
  struct Inst* pc;
};
struct Thread thread(struct Inst* pc) {
struct Thread t = {pc};
  return t;
}
struct ThreadList {
  int n;
  struct Thread t[1];
};
struct ThreadList* threadlist(int len) {
  return malloc(sizeof(struct ThreadList) + len * sizeof(struct Thread));
}
void addthread(struct ThreadList* l, struct Thread t) {
  l->t[l->n++] = t;
  switch(t.pc->opcode) {
    case Split:
      addthread(l, thread(t.pc->x));
      addthread(l, thread(t.pc->y));
      break;
    case Jmp:
      addthread(l, thread(t.pc->x));
      break;
    case Save:
      addthread(l, thread(t.pc + 1));
      break;
  }
}
int is_match(struct Prog* prog,const char* input) {
  const char* sp;
  struct Inst* pc;
  int matched = 0;
  struct ThreadList *clist, *nlist, *tlist;
  int len = prog->len;
  clist = threadlist(len);
  nlist = threadlist(len);
  addthread(clist, thread(prog->start));
  for(sp = input; ; sp++) {
    if(clist->n == 0)
      break;
    int i;
    for(i = 0; i < clist->n; i++) {
      pc = clist->t[i].pc;
      switch(pc->opcode) {
        case Char:
          if(*sp != pc->c)
            break;
          //printf("%c", pc->c);
          addthread(nlist, thread(pc+1));
          break;
        case Match:
          matched = 1;
          goto BreakFor;
       }
     }
    //printf("\n");
    BreakFor:
             tlist = clist;
	     clist = nlist;
	     nlist = tlist;
	     nlist->n = 0;
	     if(sp == '\0')
	       break;
  }
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
    case Alt: {
      char lbuf[80], rbuf[80];
      reg_to_str(lbuf, re->left);
      reg_to_str(rbuf, re->right);
      sprintf(str, "Alt(%s, %s)", lbuf, rbuf);
      break;
      }
    case Plus: {
      char buf[80];
      reg_to_str(buf, re->left);
      sprintf(str, "Plus(%s)", buf);
      break;
      }
    case Star: {
      char buf[80];
      reg_to_str(buf, re->left);
      sprintf(str, "Star(%s)", buf);
      break;
      }
    case Paren: {
      char buf[80];
      reg_to_str(buf, re->left);
      sprintf(str, "Paren(%d, %s)", re->n, buf);
      break;
      }
    case Quest: {
      char buf[80];
      reg_to_str(buf, re->left);
      sprintf(str, "Quest(%s)", buf);
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
  char buf[200];
  prog_to_str(buf, prog);
  printf("%s", buf);
  int i;
  for(i = 2; i < argc; i++) {
    if(is_match(prog, argv[i]))
      printf("match:%s\n", argv[i]);
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
  assert(3 == re_size());
reg_to_str(str, re2);
assert(!strcmp("Cat(Lit(a), Lit(b))", str));
}

void test_parse_plus(void) {
struct Regexp* re = parse("a+");
char str[80];
reg_to_str(str, re);
assert(!strcmp("Plus(Lit(a))", str));
}

void test_parse_star(void) {
struct Regexp* re = parse("a*");
char str[80];
reg_to_str(str, re);
//printf("%s\n", str);
assert(!strcmp("Star(Lit(a))", str));
}

void test_parse_alt(void) {
struct Regexp* re = parse("a|b");
char str[80];
reg_to_str(str, re);
//printf("%s\n", str);
assert(!strcmp("Alt(Lit(a), Lit(b))", str));
}

void test_parse_paren(void) {
struct Regexp* re = parse("(a)");
char str[80];
reg_to_str(str, re);
//printf("%s\n", str);
assert(!strcmp("Paren(1, Lit(a))", str));
}

void test_parse_quest(void) {
struct Regexp* re = parse("a?");
char str[80];
reg_to_str(str, re);
//printf("%s\n", str);
assert(!strcmp("Quest(Lit(a))", str));
}

void test_compile_concat(void) {
  struct Regexp* re = parse("ab");
  assert(3 == re_size());
  struct Prog* prog = compile(re);
  char str[80];
  prog_to_str(str, prog);
  char expect[] =
  "0. char a\n"
  "1. char b\n"
  "2. match\n"
  ;
  assert(!strcmp(expect, str));
  struct Inst* pc;
  int i, size;
  size = re_size();
  free(prog);
}

void test_compile_plus(void) {
  struct Regexp* re1 = parse("a+");
  struct Prog* prog = compile(re1);
  char str[80];
  prog_to_str(str, prog);
//  printf("%s", str);
  char expect[] =
  "0. char a\n"
  "1. split 0, 2\n"
  "2. match\n"
  ;
  assert(!strcmp(expect, str));
  free(prog);
}

void test_compile_star(void) {
  struct Regexp* re1 = parse("a*");
  struct Prog* prog = compile(re1);
  char str[80];

prog_to_str(str, prog);
//  printf("%s", str);
  char expect[] =
  "0. split 1, 3\n"
  "1. char a\n"
  "2. jmp 0\n"
  "3. match\n"
  ;
  assert(!strcmp(expect, str));
  //printf("count=%d\n", re_size());
  free(prog);
}

void test_alt(void) {
  struct Regexp* re1 = parse("a|b");
  struct Prog* prog = compile(re1);
  char str[80];
  prog_to_str(str, prog);
//  printf("%s", str);
  char expect[] =
  "0. split 1, 3\n"
  "1. char a\n"
  "2. jmp 4\n"
  "3. char b\n"
  "4. match\n"
  ;
  assert(!strcmp(expect, str));
  assert(is_match(prog, "a"));
  assert(is_match(prog, "b"));
  assert(!is_match(prog, "c"));
  free(prog);
}

void test_compile_paren(void) {
  struct Regexp* re1 = parse("(a)");
  struct Prog* prog = compile(re1);
  char str[80];
  prog_to_str(str, prog);
//  printf("%s", str);
  char expect[] =
  "0. save [2]\n"
  "1. char a\n"
  "2. save [3]\n"
  "3. match\n"
  ;
  assert(!strcmp(expect, str));
  free(prog);
}

void test_is_match_concat(void) {
  char input[] = "ab";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  assert(is_match(prog, input));
  assert(!is_match(prog, "bc"));
  free(prog);
}
void test_add_thread(void) {
  struct Regexp* re = parse("abcd");
  struct Prog* prog = compile(re);
  struct Inst* pc = prog->start;
  struct ThreadList* list = threadlist(4);
  addthread(list, thread(pc));
  addthread(list, thread(pc+1));
  addthread(list, thread(pc+2));
  addthread(list, thread(pc+3));
assert('a' == list->t[0].pc->c);
assert(Char == list->t[0].pc->opcode);
assert('b' == list->t[1].pc->c);
assert(Char == list->t[1].pc->opcode);
assert('c' == list->t[2].pc->c);
assert(Char == list->t[2].pc->opcode);
assert('d' == list->t[3].pc->c);
assert(Char == list->t[3].pc->opcode);
}
void test_is_match_plus(void) {
  char input[] = "a+b+";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  assert(!is_match(prog, "a"));
  assert(is_match(prog, "aab"));
  assert(is_match(prog, "aaabb"));
  assert(!is_match(prog, "b"));
}

void test_is_match_star(void) {
  char input[] = "a*b";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  assert(is_match(prog, "b"));
  assert(is_match(prog, "aaab"));
  assert(!is_match(prog, "aa"));
}

void test_is_match_paren(void) {
  struct Regexp* re = parse("P(ython|erl)");
  struct Prog* prog = compile(re);
  assert(is_match(prog, "Python"));
  assert(is_match(prog, "Perl"));
  assert(!is_match(prog, "Ruby"));
}

void test(void) {
  test_reg();
  test_parse_concat();
  test_parse_plus();
  test_parse_star();
  test_parse_alt();
  test_parse_paren();
  test_parse_quest();
  test_compile_concat();
  test_compile_plus();
  test_compile_star();
  test_alt();
  test_compile_paren();
  test_add_thread();
  test_is_match_concat();
  test_is_match_plus();
  test_is_match_star();
  test_is_match_paren();
}
