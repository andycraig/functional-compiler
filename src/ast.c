#include "ast.h"
#include "ll.h"
#include <stdlib.h>
#include <string.h>

AST *make_listStart() {
  AST *e = (AST *)malloc(sizeof(AST));
  e->tag = list_start_token;
  e->symbol_table = make_map(str_eq);
  return e;
}

AST *make_integerExp(int val) {
  AST *e = (AST *)malloc(sizeof(AST));
  e->tag = integer_exp;
  e->content.integerExp = val;
  e->symbol_table = make_map(str_eq);
  return e;
}

/**
 * Allocates memory for its name.
 */
AST *make_varExp(char *name) {
  AST *e = (AST *)malloc(sizeof(AST));
  e->tag = var_exp;
  e->content.varExp = malloc(sizeof(SVarExp));
  e->content.varExp->name =
      malloc((strlen(name) + 1) * sizeof(*e->content.varExp->name));
  strcpy(e->content.varExp->name, name);
  e->content.varExp->is_recursive = 0;
  e->symbol_table = make_map(str_eq);
  return e;
}

AST *make_makeClosureExp(char *name, int n_bound_vars, int n_free_vars) {
  AST *e = malloc(sizeof(AST));
  e->tag = make_closure_exp;
  e->content.makeClosureExp = malloc(sizeof(SMakeClosureExp));
  e->content.makeClosureExp->name =
      malloc((strlen(name) + 1) * sizeof(*e->content.makeClosureExp->name));
  strcpy(e->content.makeClosureExp->name, name);
  e->content.makeClosureExp->n_bound_vars = n_bound_vars;
  e->content.makeClosureExp->n_free_vars = n_free_vars;
  e->content.makeClosureExp->free_vars = make_list();
  e->symbol_table = make_map(str_eq);
  return e;
}

AST *make_listExp() {
  AST *e = (AST *)malloc(sizeof(AST));
  e->tag = list_exp;
  e->content.listExp = (SListExp *)malloc(sizeof(SListExp));
  e->content.listExp->rest = make_list();
  e->symbol_table = make_map(str_eq);
  return e;
}

AST *make_globalExp() {
  AST *e = (AST *)malloc(sizeof(AST));
  e->tag = global_exp;
  e->content.globalExp = (SGlobalExp *)malloc(sizeof(SGlobalExp));
  e->content.globalExp->main = NULL;
  e->content.globalExp->rest = make_list();
  e->content.globalExp->standard = make_map(str_eq);
  map_insert_key(e->content.globalExp->standard, "plus");
  map_insert_key(e->content.globalExp->standard, "minus");
  map_insert_key(e->content.globalExp->standard, "equals");
  e->symbol_table = make_map(str_eq);
  e->parent = NULL;
  return e;
}

AST *make_lambdaExp(Map *args, AST *body) {
  AST *e = (AST *)malloc(sizeof(AST));
  e->tag = lambda_exp;
  e->content.lambdaExp = (SLambdaExp *)malloc(sizeof(SLambdaExp));
  e->content.lambdaExp->args = args;
  e->content.lambdaExp->body = body;
  e->symbol_table = make_map(str_eq);
  return e;
}

AST *make_letExp(char *arg, AST *defn, AST *body, short is_recursive) {
  AST *e = (AST *)malloc(sizeof(AST));
  e->tag = let_exp;
  e->content.letExp = (SLetExp *)malloc(sizeof(SLetExp));
  e->content.letExp->arg = arg;
  e->content.letExp->defn = defn;
  e->content.letExp->body = body;
  e->content.letExp->is_recursive = is_recursive;
  e->symbol_table = make_map(str_eq);
  return e;
}

AST *make_ifExp(AST *pred, AST *case_true, AST *case_false) {
  AST *e = (AST *)malloc(sizeof(AST));
  e->tag = if_exp;
  e->content.ifExp = (SIfExp *)malloc(sizeof(SIfExp));
  e->content.ifExp->pred = pred;
  e->content.ifExp->case_true = case_true;
  e->content.ifExp->case_false = case_false;
  e->symbol_table = make_map(str_eq);
  return e;
}

void free_ast_node(AST *node) {
  if (node->tag == var_exp) {
    free(node->content.varExp->name);
  } else if (node->tag == make_closure_exp) {
    free(node->content.makeClosureExp->name);
  }
  // TODO Deal with symbol_table
  free(node);
}

void free_ast(AST *ast) {
  // Free children first, so do via post-order traversal
  LL *stack = get_post_order_ast(ast);
  while (stack->len > 0) {
    AST *node = pop_head(stack);
    free_ast_node(node);
  }
  free_list(stack);
}

int get_n_children(AST *ast) {
  switch (ast->tag) {
  case list_exp:
    return 1 + ast->content.listExp->rest->len;
    break;
  case global_exp:
    return 1 + ast->content.globalExp->rest->len;
    break;
  case lambda_exp:
    return 1; // body
    break;
  case if_exp:
    return 3; // pred, case_true, case_false
    break;
  case let_exp:
    return 2; // defn, body
    break;
  default:
    return 0;
    break;
  }
}

/**
 * Params:
 *  nth: 0-indexed
 */
AST *get_child(AST *node, int nth) {
  AST *child;
  // TODO Raise error if nth >= get_n_children(node)
  switch (node->tag) {
  case list_exp:
    child = (nth == 0) ? node->content.listExp->first
                       : get_i(node->content.listExp->rest, nth - 1);
    break;
  case global_exp:
    child = (nth == 0) ? node->content.globalExp->main
                       : get_i(node->content.globalExp->rest, nth - 1);
    break;
  case lambda_exp:
    child = node->content.lambdaExp->body;
    break;
  case if_exp:
    switch (nth) {
    case 0:
      child = node->content.ifExp->pred;
      break;
    case 1:
      child = node->content.ifExp->case_true;
      break;
    case 2:
      child = node->content.ifExp->case_false;
      break;
    }
    break;
  case let_exp:
    switch (nth) {
    case 0:
      child = node->content.letExp->defn;
      break;
    case 1:
      child = node->content.letExp->body;
      break;
    }
    break;
  default:
    printf("ERROR! Unexpected tag in get_child\n");
    // printf("Tag: %d\n", node->tag);
    break;
  }
  return child;
}

/**
 * Returns:
 *  Stack which pops to yield post-order traversal of AST
 */
LL *get_post_order_ast(AST *ast) {
  LL *build_stack = make_list();
  LL *result_stack = make_list();
  push_head(build_stack, ast);
  while (build_stack->len > 0) {
    AST *top = (AST *)pop_head(build_stack);
    for (int i_child = 0; i_child < get_n_children(top); ++i_child) {
      AST *child = get_child(top, i_child);
      push_head(build_stack, child);
    }
    push_head(result_stack, top);
  }
  free_list(build_stack);
  return result_stack;
}

void JSONify_AST(AST *ast) { JSONify_AST_aux(ast, 0); }

void JSONify_AST_aux(AST *ast, int depth) {
  indent(depth);
  printf("{\n");
  ++depth;
  indent(depth);
  printf("\"Location\": \"%p\", ", ast);
  printf("\"Parent\": \"%p\", ", ast->parent);
  switch (ast->tag) {
  case list_start_token:
    indent(depth);
    printf("LIST START TOKEN");
    break;
  case integer_exp:
    printf("\"tag\": \"integer_exp\",\n");
    indent(depth);
    JSONify_symbol_table(ast->symbol_table);
    indent(depth);
    printf("\"content\": %d\n", ast->content.integerExp);
    break;
  case var_exp:
    printf("\"tag\": \"var_exp\",\n");
    indent(depth);
    JSONify_symbol_table(ast->symbol_table);
    indent(depth);
    printf("\"name\": \"%s\",\n", ast->content.varExp->name);
    indent(depth);
    printf("\"is_recursive\": %d\n", ast->content.varExp->is_recursive);
    break;
  case if_exp:
    printf("\"tag\": \"if_exp\",\n");
    indent(depth);
    JSONify_symbol_table(ast->symbol_table);
    indent(depth);
    printf("\"pred\":\n");
    JSONify_AST_aux(ast->content.ifExp->pred, depth);
    indent(depth);
    printf(", \"case_true\":\n");
    JSONify_AST_aux(ast->content.ifExp->case_true, depth);
    indent(depth);
    printf(", \"case_false\":\n");
    JSONify_AST_aux(ast->content.ifExp->case_false, depth);
    break;
  case lambda_exp:
    printf("\"tag\": \"lambda_exp\",\n");
    indent(depth);
    JSONify_symbol_table(ast->symbol_table);
    indent(depth);
    printf("\"name\": \"%s\",\n", ast->content.lambdaExp->name);
    indent(depth);
    printf("\"args\": ");
    map_JSONify(ast->content.lambdaExp->args);
    printf(",\n");
    indent(depth);
    printf("\"body\":\n");
    JSONify_AST_aux(ast->content.lambdaExp->body, depth);
    break;
  case let_exp:
    printf("\"tag\": \"let_exp\",\n");
    indent(depth);
    JSONify_symbol_table(ast->symbol_table);
    indent(depth);
    printf("\"is_recursive\": %d,\n", ast->content.letExp->is_recursive);
    indent(depth);
    printf("\"arg\": \"%s\",\n", ast->content.letExp->arg);
    indent(depth);
    printf("\"defn\":\n");
    JSONify_AST_aux(ast->content.letExp->defn, depth);
    indent(depth);
    printf(", \"body\":\n");
    JSONify_AST_aux(ast->content.letExp->body, depth);
    break;
  case list_exp:
    printf("\"tag\": \"list_exp\",\n");
    indent(depth);
    JSONify_symbol_table(ast->symbol_table);
    indent(depth);
    printf("\"first\":\n");
    JSONify_AST_aux(ast->content.listExp->first, depth);
    printf(",\n");
    indent(depth);
    printf("\"rest\": [\n");
    for (int i_exp = 0; i_exp < ast->content.listExp->rest->len; ++i_exp) {
      JSONify_AST_aux((AST *)get_i(ast->content.listExp->rest, i_exp),
                      depth + 1);
      if (i_exp < ast->content.listExp->rest->len - 1) {
        printf(",\n");
      }
    }
    printf("]\n");
    break;
  case global_exp:
    printf("\"tag\": \"global_exp\",\n");
    indent(depth);
    JSONify_symbol_table(ast->symbol_table);
    indent(depth);
    printf("\"main\":\n");
    JSONify_AST_aux(ast->content.globalExp->main, depth);
    printf(",\n");
    indent(depth);
    printf("\"rest\": [ ");
    for (int i_exp = 0; i_exp < ast->content.globalExp->rest->len; ++i_exp) {
      JSONify_AST_aux((AST *)get_i(ast->content.globalExp->rest, i_exp), depth);
      if (i_exp + 1 < ast->content.makeClosureExp->free_vars->len) {
        printf(",");
      }
    }
    indent(depth);
    printf("]\n");
    break;
  case make_closure_exp:
    printf("\"tag\": \"make_closure_exp\",\n");
    indent(depth);
    JSONify_symbol_table(ast->symbol_table);
    indent(depth);
    printf("\"name\":\"%s\",\n", ast->content.makeClosureExp->name);
    indent(depth);
    printf("\"n_bound_vars\":\"%d\",\n",
           ast->content.makeClosureExp->n_bound_vars);
    indent(depth);
    printf("\"n_free_vars\":\"%d\",\n",
           ast->content.makeClosureExp->n_free_vars);
    indent(depth);
    printf("\"free_vars\": [ ");
    for (int i_exp = 0; i_exp < ast->content.makeClosureExp->free_vars->len;
         ++i_exp) {
      JSONify_AST_aux(
          (AST *)get_i(ast->content.makeClosureExp->free_vars, i_exp), depth);
      if (i_exp + 1 < ast->content.makeClosureExp->free_vars->len) {
        printf(",");
      }
    }
    indent(depth);
    printf("]\n");
    break;
  default:
    indent(depth);
    printf("UNKNOWN TAG: %d", ast->tag);
    break;
  }
  --depth;
  indent(depth);
  printf("}");
}

void JSONify_symbol_table(Map *s) {
  printf("\"symbol table\": ");
  if (s) {
    map_JSONify(s);
  }
  printf(",\n");
}

int str_eq(void *x, void *y) { return strcmp((char *)x, (char *)y) == 0; }

void indent(int n) {
  for (int i = 0; i < n; ++i) {
    printf("\t");
  }
}

int is_ancestor(AST *candidate_ancestor, AST *candidate_descendent) {
  if (candidate_ancestor->parent == candidate_descendent) {
    return 1;
  } else if (candidate_descendent->tag == global_exp) {
    return 0;
  } else {
    return is_ancestor(candidate_ancestor, candidate_descendent->parent);
  }
}
