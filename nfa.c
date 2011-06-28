#include "regexp.h"
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
int main(void) {
  test();
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
char str[80];
reg_to_str(str, re1);
assert(!strcmp("Lit(a)", str));
struct Regexp* re2 = parse("ab");
reg_to_str(str, re2);
assert(!strcmp("Cat(Lit(a), Lit(b))", str));
}

void test(void) {
  test_reg();
  test_parse_concat();
}
