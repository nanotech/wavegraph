typedef struct list {
  void *head;
  struct list *tail;
} list;

list *list_copy(list l);
void list_mutcons(void *, list **);
void *list_foldl(void *(*)(void *, void *), void *acc, const list *);
void *list_foldr(void *(*)(void *, void *), void *acc, const list *);
void list_mapM_(void (*)(void *), const list *);
void list_freerec(list *l);
//list *list_cons(void *, list *);
