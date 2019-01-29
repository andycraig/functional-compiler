#include "ll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * When finished with, use free_list to clean up.
 */
LL *make_list() {
  LL *list = (LL *)malloc(sizeof(LL));
  list->head = NULL;
  list->tail = NULL;
  list->len = 0;
  return list;
}

void free_list(LL *list) {
  // Free all nodes
  while (list->len > 0) {
    pop_head(list);
  }
  free(list);
}

int push_tail(LL *list, void *source) {
  LLNode *new_node = (LLNode *)malloc(sizeof(LLNode));
  new_node->val = source;
  new_node->next = NULL;
  if (list->len == 0) {
    list->head = new_node;
    list->tail = new_node;
    ++list->len;
  } else {
    list->tail->next = new_node;
    ++list->len;
    list->tail = list->tail->next;
  }
  return 0;
}

int push_head(LL *list, void *source) {
  LLNode *new_node = (LLNode *)malloc(sizeof(LLNode));
  new_node->val = source;
  new_node->next = NULL;
  if (list->len == 0) {
    list->head = new_node;
    list->tail = new_node;
    ++list->len;
  } else {
    new_node->next = list->head;
    list->head = new_node;
    ++list->len;
  }
  return 0;
}

void *pop_head(LL *list) {
  void *result;
  LLNode *old = list->head;
  result = list->head->val;
  if (list->len == 1) {
    list->head = NULL;
    list->tail = NULL;
  } else {  // At least one other element left
    list->head = list->head->next;
  }
  list->len--;
  free(old);
  return result;
}

void *pop_tail(LL *list) {
  void *result;
  LLNode *old = list->tail;
  result = list->tail->val;
  if (list->len == 1) {
    list->head = NULL;
    list->tail = NULL;
  } else {  // At least one other element left
    // Find tail
    LLNode *current = list->head;
    while (current->next->next) {
      current = current->next;
    }
    current->next = NULL;
    list->tail = current;
  }
  list->len--;
  free(old);
  return result;
}

void ll_print(LL *list) {
  LLNode *current = list->head;
  while (current) {
    printf("%s\n", (char *)current->val);
    current = current->next;
  }
  printf("\n");
}

void *get_i(LL *list, int n) {
  LLNode *current = list->head;
  for (int i = 0; i < n; ++i) {
    current = current->next;
  }
  return current->val;
}

Map *make_map(int (*eq)(void *, void *)) {
  Map *map = (Map *)malloc(sizeof(Map));
  map->list = make_list();
  map->eq = eq;
  return map;
}

void map_insert_key(Map *map, void *key) {
  if (!map_in(map, key)) {
    Tuple *tuple = make_tuple();
    tuple->first = key;
    push_tail(map->list, tuple);
  }
}

void map_insert_value(Map *map, void *key, void *value) {
  Tuple *key_in_map = map_find(map, key);
  if (key_in_map) {
    key_in_map->second = value;
  } else {
    map_insert_key(map, key);
    // Now new key is at tail
    Tuple *tuple = (Tuple *)map->list->tail->val;
    tuple->second = value;
  }
}

Tuple *map_find(Map *map, void *key) {
  LLNode *current = map->list->head;
  while (current) {
    Tuple *tuple = (Tuple *)current->val;
    if (map->eq(tuple->first, key)) {
      return current->val;
    } else {
      current = current->next;
    }
  }
  return NULL;
}

int map_in(Map *map, void *key) {
  Tuple *key_val = map_find(map, key);
  return key_val ? 1 : 0;
}

void *map_get(Map *map, void *key) {
  Tuple *key_value = map_find(map, key);
  return key_value->second;
}

void *get_key_i(Map *map, int n) {
  Tuple *key_val = get_i(map->list, n);
  return key_val->first;
}

Tuple *make_tuple() {
  Tuple *t = (Tuple *)malloc(sizeof(Tuple));
  t->first = NULL;
  t->second = NULL;
  return t;
}

void map_JSONify(Map *map) {
  printf("[");
  for (int i_var = 0; i_var < map->list->len; ++i_var) {
    Tuple *var_val = (Tuple *)get_i(map->list, i_var);
    char *var = (char *)var_val->first;
    if (var_val->second) {
      int *val = (int *)var_val->second;
      printf("%s:%d ", var, *val);
    } else {
      printf("%s:EMPTY ", var);
    }
  }
  printf("]");
}