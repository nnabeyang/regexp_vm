#include "regexp.h"
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
static void addthread(struct ThreadList* l, struct Thread t) {
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
int is_match_thompson(struct Prog* prog,char* input) {
  char* sp;
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
	case Any:
	  if(*sp == '\0')
	    break;
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
  if(matched)
    return sp - input;
            return 0;
}

static void test_add_thread(void) {
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

static void test_dot(void) {
  struct Regexp* re = parse("a.+");
  struct Prog* prog = compile(re);
  assert(!is_match_thompson(prog, "a"));
  assert(is_match_thompson(prog, "aa"));
  assert(is_match_thompson(prog, "ab"));
}

static void test_alt(void) {
  struct Regexp* re = parse("a|b");
  struct Prog* prog = compile(re);
  assert(is_match_thompson(prog, "a"));
  assert(is_match_thompson(prog, "b"));
  assert(!is_match_thompson(prog, "c"));
}

static void test_quest(void) {
  struct Regexp* re = parse("ba?");
  struct Prog* prog = compile(re);
  assert(2 == is_match_thompson(prog, "b"));
  assert(3 == is_match_thompson(prog, "ba"));
  assert(3 == is_match_thompson(prog, "baa"));
  assert(3 == is_match_thompson(prog, "baaaaa"));
  assert(!is_match_thompson(prog, "aaaaab"));
}

static void test_is_match_thompson_concat(void) {
  char input[] = "ab";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  assert(is_match_thompson(prog, input));
  assert(!is_match_thompson(prog, "bc"));
  free(prog);
}

static void test_is_match_thompson_plus(void) {
  char input[] = "a+b+";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  assert(!is_match_thompson(prog, "a"));
  assert(is_match_thompson(prog, "aab"));
  assert(is_match_thompson(prog, "aaabb"));
  assert(!is_match_thompson(prog, "b"));
}

static void test_is_match_thompson_star(void) {
  char input[] = "a*b";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  assert(is_match_thompson(prog, "b"));
  assert(is_match_thompson(prog, "aaab"));
  assert(!is_match_thompson(prog, "aa"));
  free(prog);
}

static void test_is_match_thompson_paren(void) {
  struct Regexp* re = parse("P(ython|erl)");
  struct Prog* prog = compile(re);
  assert(is_match_thompson(prog, "Python"));
  assert(is_match_thompson(prog, "Perl"));
  assert(!is_match_thompson(prog, "Ruby"));
  free(prog);
}

void test_thompson(void) {
  test_add_thread();
  test_dot();
  test_alt();
  test_quest();
  test_is_match_thompson_concat();
  test_is_match_thompson_plus();
  test_is_match_thompson_star();  
  test_is_match_thompson_paren();
}
