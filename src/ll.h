#ifndef LL_H
#define LL_H

#define SYMBOL_MAX_LENGTH 16

typedef struct LLNode {
  struct LLNode *next;
  void *val;
} LLNode;

typedef struct LL {
  LLNode *head;
  LLNode *tail;
  int len;
} LL;

// Very inefficient linked list-backed map
typedef struct Map {
  LL *list;  // Linked list of Tuples
  int (*eq)(void *, void *);
} Map;

typedef struct Tuple {
  void *first;
  void *second;
} Tuple;

LL *make_list();

void free_list(LL *list);

int push_tail(LL *list, void *val);

int push_head(LL *list, void *val);

void *pop_head(LL *list);

void *pop_tail(LL *list);

void ll_print(LL *list);

void *get_i(LL *list, int n);

Map *make_map(int (*eq)(void *, void *));

void map_insert_key(Map *map, void *key);

void map_insert_value(Map *map, void *key, void *value);

Tuple *map_find(Map *map, void *key);

int map_in(Map *map, void *key);

void *map_get(Map *map, void *key);

void *get_key_i(Map *map, int n);

Tuple *make_tuple();

void map_JSONify(Map *map);

#endif
