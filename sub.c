#include "regexp.h"
#define MAXSUB 20
struct Sub {
  int ref;
  char* sub[MAXSUB];
};
struct Sub* freesub;
struct Sub* newsub(void) {
  struct Sub* s;
  assert(freesub == NULL);
  s = malloc(sizeof(struct Sub));
  s->ref = 0;
  return s;
}
void incref(struct Sub* s) {
  s->ref++;
}
void decref(struct Sub* s) {
  if(--s->ref == 0) {
    // new freesub has a previous  freesub in the field of sub[0]
    s->sub[0] = (char*)freesub;
    freesub = s;
  }
}
void test_sub_ref(void) {
  struct Sub* s = newsub();
  assert(0 == s->ref);
  incref(s);
  assert(1 == s->ref);
  incref(s);
  assert(2 == s->ref);
  decref(s);
  assert(1 == s->ref);
  decref(s);
  assert(s == freesub);
  assert(s->sub[0] == NULL);
}
void test_sub(void) {
  test_sub_ref();
}
