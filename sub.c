#include "regexp.h"
struct Sub* freesub;
struct Sub* newsub(void) {
  struct Sub* s;
  if(freesub == NULL)
    s = malloc(sizeof(struct Sub));
  else {
    s = freesub;
    freesub = (struct Sub*)freesub->sub[0];// freesub -> prev freesub
  }
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
struct Sub* update(struct Sub* s, int i, char* c) {
struct Sub* s1;
  if(s->ref > 1) {
    s1 = newsub();
    int i;
    for(i = 0; i < MAXSUB; i++)
      s1->sub[i] = s->sub[i];
    s = s1;
  }
  s->sub[i] = c;
  return s;
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

void test_sub_update(void) {
  struct Sub *s1, *s2;
  s1 = newsub();
  char str[] = "hello";
  char* p = str;
  int i;
  incref(s1);
  s2 = update(s1, 0, p++);
  assert(s1->sub[0] == &str[0]);
  assert(s2->sub[0] == &str[0]);
  s1->sub[0] = p++;
  assert(s1->sub[0] == &str[1]);
  assert(s2->sub[0] == &str[1]);
  incref(s1);
  assert(2 == s1->ref);
  s2 = update(s1, 0, p++);// copy
  assert(s1->sub[0] == &str[1]);
  assert(s2->sub[0] == &str[2]);
  s1->sub[0] = p;
  assert(s1->sub[0] == &str[3]);
  assert(s2->sub[0] == &str[2]);
  free(s1);
  free(s2);
}

void test_sub(void) {
  test_sub_ref();
  test_sub_update();
}
