#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
struct Regexp {
  int type;
  int ch;
  struct Regexp* left;
  struct Regexp* right;
};
enum {
  Lit = 1,
  Cat,
};

struct Regexp* reg(int type, struct Regexp* left, struct Regexp* right);
struct Regexp* parse(const char* source);
int re_size(void);
