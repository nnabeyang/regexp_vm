#include "regexp.h"
static int gen;
struct Thread {
  struct Inst* pc;
  struct Sub* sub;
};
static struct Thread thread(struct Inst* pc, struct Sub* sub) {
struct Thread t = {pc, sub};
  return t;
}
struct ThreadList {
  int n;
  struct Thread t[1];
};
static struct ThreadList* threadlist(int len) {
  return malloc(sizeof(struct ThreadList) + len * sizeof(struct Thread));
}
static void addthread(struct ThreadList* l, struct Thread t, char* sp) {
  if(t.pc->gen == gen)
    return;
  t.pc->gen = gen;
  switch(t.pc->opcode) {
    default:
      l->t[l->n++] = t;
      break;
    case Split:
      incref(t.sub);
      addthread(l, thread(t.pc->x, t.sub), sp);
      addthread(l, thread(t.pc->y, t.sub), sp);
      break;
    case Jmp:
      addthread(l, thread(t.pc->x, t.sub), sp);
      break;
    case Save:
      addthread(l, thread(t.pc + 1, update(t.sub, t.pc->n-2, sp)), sp);
      break;
  }
}

int is_match_pike(struct Prog* prog,char* input, char** subp) {
  char* sp;
  struct Sub* sub;
  struct Inst* pc;
  int matched = 0;
  struct ThreadList *clist, *nlist, *tlist;
  int len = prog->len;
  clist = threadlist(len);
  nlist = threadlist(len);
  gen++;
  sub = newsub();
  int i;
  for(i = 0; i < MAXSUB; i++)
    sub->sub[i] = NULL;
  addthread(clist, thread(prog->start, sub), input);
  for(sp = input; ; sp++) {
    if(clist->n == 0)
      break;
    gen++;
    for(i = 0; i < clist->n; i++) {
      pc = clist->t[i].pc;
      sub = clist->t[i].sub;
      switch(pc->opcode) {
        case Char:
          if(*sp != pc->c) {
            decref(sub);
	    break;
	  }
          //printf("%c", pc->c);
	case Any:
	  if(*sp == '\0') {
	    decref(sub);
	    break;
	  }
          addthread(nlist, thread(pc+1, sub), sp+1);
          break;
        case Match:
          matched = 1;
	  int i;
	  for(i = 0; i < MAXSUB; i++)
	    subp[i] = sub->sub[i];
          for(i = 0; i < clist->n; i++)
	    decref(clist->t[i].sub);
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
  char input[] = "ab";
  struct Regexp* re = parse(input);
  struct Prog* prog = compile(re);
  struct Inst* pc = prog->start;
  struct Sub* s = newsub();
  struct ThreadList* list = threadlist(2);
  gen++;
  addthread(list, thread(pc, s), input);
  gen++;
  addthread(list, thread(pc+1, s), input);
  assert('a' == list->t[0].pc->c);
  assert(Char == list->t[0].pc->opcode);
  assert('b' == list->t[1].pc->c);
  assert(Char == list->t[0].pc->opcode);
}

static void test_submatch(void) {
  struct Regexp* re = parse("G(o+)gle");
  struct Prog* prog = compile(re);
  char* sub[MAXSUB];
  char str[] = "Google";
  assert(is_match_pike(prog, str, sub));
  char buf[80], *pbuf;
  pbuf = buf;
  int i;
  for(i = MAXSUB-1; i >= 0; i--)
    if(sub[i])
      break;
  assert(i == 1);
  char* p = sub[i-1];
  while(p != sub[i])
    *pbuf++ = *p++;
  *pbuf = '\0';
  assert(!strcmp("oo", buf));
}
/*
static void test_alt(void) {
  struct Regexp* re = parse("(.+)world");
  struct Prog* prog = compile(re);
  char* sub[MAXSUB];
  char str[] = "helloworld";
  assert(is_match_pike(prog, str, sub));
  char buf[80], *pbuf;
  pbuf = buf;
  char* p = str;
  while(p != sub[1])
    *pbuf++ = *p++;
  *pbuf = '\0';
  assert(!strcmp("hello", buf));
}*/

void test_pike(void) {
  test_add_thread();  
  test_submatch();
}
