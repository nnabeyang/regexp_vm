#include "regexp.h"
static int gen;
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
  if(t.pc->gen == gen) {
//    printf("gen=%d\n", gen);
    return;
  } else {
    t.pc->gen = gen;
  }
  switch(t.pc->opcode) {
    default:
      l->t[l->n++] = t;
      break;
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
int is_match_thompson(struct Prog* prog, char* input, char** subp) {
  char* sp;
  struct Inst* pc;
  int matched = 0;
  struct ThreadList *clist, *nlist, *tlist;
  int len = prog->len;
  clist = threadlist(len);
  nlist = threadlist(len);
  int i;
  for(i = 0; i < MAXSUB; i++)
    subp[i] = NULL;
  subp[0] = input;
  gen++;
  addthread(clist, thread(prog->start));
  for(sp = input; ; sp++) {
    if(clist->n == 0)
      break;
    gen++;
    for(i = 0; i < clist->n; i++) {
      pc = clist->t[i].pc;
      switch(pc->opcode) {
        case Char:
          if(*sp != pc->c)
            break;
         // printf("%c", *sp);
	case Any:
	  if(*sp == '\0')
	    break;
         // printf("%c", *sp);
          addthread(nlist, thread(pc+1));
          break;
        case Match:
          matched = 1;
	  subp[1] = sp;
          goto BreakFor;
       }
     }
   // printf("\n");
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
  gen++;
  addthread(list, thread(pc));
  gen++;
  addthread(list, thread(pc+1));
  gen++;
  addthread(list, thread(pc+2));
  gen++;
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
  //char buf[80];
  //reg_to_str(buf, re);
  //printf("%s\n", buf);
  struct Prog* prog = compile(re);
  //print_prog(prog);
  char* sub[MAXSUB];
  assert(!is_match_thompson(prog, "a", sub));
  char* input = "aa";
  assert(is_match_thompson(prog, input, sub));
  assert(0 == (int)(sub[0] - input));
  assert(2 == (int)(sub[1] - input));
  assert(is_match_thompson(prog, "ab", sub));
}

static void test_alt(void) {
  struct Regexp* re = parse("a|b");
  struct Prog* prog = compile(re);
  char* sub[MAXSUB];
  assert(is_match_thompson(prog, "a", sub));
  assert(is_match_thompson(prog, "b", sub));
  assert(!is_match_thompson(prog, "c", sub));
}

static void test_quest(void) {
  struct Regexp* re = parse("ba?");
  struct Prog* prog = compile(re);
  char* sub[MAXSUB];
  assert(2 == is_match_thompson(prog, "b", sub));
  assert(3 == is_match_thompson(prog, "ba", sub));
  assert(3 == is_match_thompson(prog, "baa", sub));
  assert(3 == is_match_thompson(prog, "baaaaa", sub));
  assert(!is_match_thompson(prog, "aaaaab", sub));
}

static void test_is_match_thompson_concat(void) {
  struct Regexp* re = parse("ab");
  struct Prog* prog = compile(re);
  char* sub[MAXSUB];
  assert(is_match_thompson(prog, "ab", sub));
  assert(!is_match_thompson(prog, "bc", sub));
  free(prog);
}

static void test_is_match_thompson_plus(void) {
  char input[] = "a+b+";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  char* sub[MAXSUB];
  assert(!is_match_thompson(prog, "a", sub));
  assert(is_match_thompson(prog, "aab", sub));
  assert(is_match_thompson(prog, "aaabb", sub));
  assert(!is_match_thompson(prog, "b", sub));
}

static void test_is_match_thompson_star(void) {
  char input[] = "a*b";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  char* sub[MAXSUB];
  assert(is_match_thompson(prog, "b", sub));
  assert(is_match_thompson(prog, "aaab", sub));
  assert(!is_match_thompson(prog, "aa", sub));
  free(prog);
}

static void test_is_match_thompson_paren(void) {
  struct Regexp* re = parse("P(ython|erl)");
  struct Prog* prog = compile(re);
  char* sub[MAXSUB];
  assert(is_match_thompson(prog, "Python", sub));
  assert(is_match_thompson(prog, "Perl", sub));
  assert(!is_match_thompson(prog, "Ruby", sub));
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
