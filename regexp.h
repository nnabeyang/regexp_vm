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
#define MAXSUB 20
struct Sub {
  int ref;
  char* sub[MAXSUB];
};
struct Sub* newsub(void);
void incref(struct Sub*);
void decref(struct Sub*);
struct Sub* update(struct Sub*, int, char*);
struct Regexp* reg(int type, struct Regexp* left, struct Regexp* right);
struct Regexp* parse(const char* source);
int re_size(void);
int is_match_thompson(struct Prog*,char*, char**);
int is_match_pike(struct Prog*,char*, char**);
struct Prog* compile(struct Regexp* re);
// for testing
void test_thompson(void);
void test_sub(void);
void test_pike(void);
void reg_to_str(char* str, struct Regexp* re);
void print_prog(struct Prog* p);
