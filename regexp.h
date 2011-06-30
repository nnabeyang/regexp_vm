#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
struct Regexp {
  int type;
  int ch;
  int n;// nparen
  struct Regexp* left;
  struct Regexp* right;
};
enum {
  Lit = 1,
  Cat,
  Plus,
  Star,
  Alt,
  Paren,
  Quest,
  Dot,
};
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
  Any,
  Match,
};
struct Prog {
  struct Inst* start;
  int len;
};

struct Regexp* reg(int type, struct Regexp* left, struct Regexp* right);
struct Regexp* parse(const char* source);
int re_size(void);
int is_match_thompson(struct Prog*,char*);
struct Prog* compile(struct Regexp* re);
// for testing
void test_thompson(void);
void test_sub(void);
