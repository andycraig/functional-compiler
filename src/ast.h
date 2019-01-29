#include <stdio.h>
#include <stdlib.h>
#include "ll.h"
#ifndef AST_H
#define AST_H

typedef struct SVarExp {
  char *name;
  short is_recursive;
} SVarExp;

typedef struct SIfExp {
  // (if pred case_true case_false)
  struct Exp *pred;
  struct Exp *case_true;
  struct Exp *case_false;
} SIfExp;

typedef struct SLambdaExp {
  // (lambda args body)
  char *name;  // Only set for globals
  Map *args;
  struct Exp *body;
} SLambdaExp;

typedef struct SLetExp {
  // One arg only:
  // (let (arg defn) body)
  char *arg;
  struct Exp *defn;
  struct Exp *body;
  short is_recursive;
} SLetExp;

typedef struct SDefExp {
  // One arg only:
  // (let (arg defn) body)
  char *arg;
  struct Exp *defn;
} SDefExp;

typedef struct SListExp {
  struct Exp *first;
  LL *rest;
} SListExp;

typedef struct SGlobalExp {
  struct Exp *main;
  LL *rest;
  Map *standard;
} SGlobalExp;

typedef struct SMakeClosureExp {
  char *name;
  int n_bound_vars;
  int n_free_vars;
  LL *free_vars;
} SMakeClosureExp;

typedef struct Exp {
  struct Exp *parent;
  enum {
    list_start_token,  // 0 Shouldn't appear in the AST, but used as a delimiter
                       // while constructing the AST
    integer_exp,       // 1
    var_exp,           // 2
    if_exp,            // 3
    lambda_exp,        // 4
    let_exp,           // 5
    list_exp,          // 6
    global_exp,        // 7
    make_closure_exp,  // 8
  } tag;
  union {
    int integerExp;
    struct SVarExp *varExp;
    struct SLambdaExp *lambdaExp;
    struct SLetExp *letExp;
    struct SIfExp *ifExp;
    struct SListExp *listExp;
    struct SGlobalExp *globalExp;
    struct SMakeClosureExp *makeClosureExp;
  } content;
  Map *symbol_table;
} AST;

AST *make_listStart();

AST *make_integerExp(int val);

AST *make_varExp(char *name);

AST *make_makeClosureExp(char *name, int n_bound_vars, int n_free_vars);

AST *make_listExp();

AST *make_letExp(char *arg, AST *defn, AST *body, short is_recursive);

AST *make_lambdaExp(Map *args, AST *body);

AST *make_ifExp(AST *pred, AST *case_true, AST *case_false);

AST *make_ifExp(AST *pred, AST *case_true, AST *case_false);

AST *make_globalExp();

void free_ast_node(AST *node);

void free_ast(AST *ast);

int get_n_children(AST *ast);

AST *get_child(AST *node, int nth);

LL *get_post_order_ast(AST *ast);

void JSONify_AST(AST *ast);

void JSONify_AST_aux(AST *ast, int depth);

void JSONify_symbol_table(Map *s);

int str_eq(void *x, void *y);

void indent(int n);

int is_ancestor(AST *candidate_ancestor, AST *candidate_descendent);

#endif