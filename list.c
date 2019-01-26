#include "list.h"
#include <stdlib.h>
#include <stdio.h>

list *list_copy(list l) {
  list *l2 = malloc(sizeof *l2);
  *l2 = l;
  return l2;
}

void list_mutcons(void *h, list **l) {
  *l = list_copy((list){h, *l});
}

void *list_foldl(void *(*f)(void *, void *), void *acc, const list *l) {
  return (l == NULL) ? acc : list_foldl(f, f(l->head, acc), l->tail);
}

void *list_foldr(void *(*f)(void *, void *), void *acc, const list *l) {
  return (l == NULL) ? acc : f(l->head, list_foldr(f, acc, l->tail));
}

void list_mapM_(void (*f)(void *), const list *l) {
  if (l != NULL) {
    f(l->head);
    list_mapM_(f, l->tail);
  }
}

void list_freerec(list *l) {
  if (l != NULL) {
    list_freerec(l->tail);
    free(l);
  }
}

/*
list list_cons(void *head, list *tail) {
  list *tailp = malloc(sizeof *tailp);
  *tailp = *tail;
  list l = { head, tailp };
  return l;
}
*/
