#include "scope.h"
#include <string.h>
#include "ast.h"

/**
 * Adds symbol tables to an AST.
 */
void make_scopes(AST *ast) {
  // Only need to add to scope if ast is a let or lambda,
  // as these are the only types that define scope.
  if (ast->tag == let_exp) {
    // Given (let arg defn body):
    // defn is still in parent scope;
    // body is in let node's scope;
    // arg is in parent's scope
    int var_str_len = strlen(ast->content.letExp->arg);
    char *scope_var = malloc(sizeof(*scope_var) * (1 + var_str_len));
    strcpy(scope_var, ast->content.letExp->arg);
    map_insert_key(ast->content.letExp->body->symbol_table, scope_var);
    if (ast->content.letExp->is_recursive) {
      char *recursive = malloc(sizeof(*recursive) * (1 + strlen("RECURSIVE")));
      strcpy(recursive, "RECURSIVE");
      map_insert_value(ast->content.letExp->defn->symbol_table, scope_var,
                       recursive);
    }
    make_scopes(ast->content.letExp->defn);
    make_scopes(ast->content.letExp->body);
  } else if (ast->tag == lambda_exp) {
    // Given (lambda vars exp),
    // add all vars to scope of exp.
    for (int i_arg = 0; i_arg < ast->content.lambdaExp->args->list->len;
         ++i_arg) {
      map_insert_key(ast->content.lambdaExp->body->symbol_table,
                     (char *)get_key_i(ast->content.lambdaExp->args, i_arg));
    }
    make_scopes(ast->content.lambdaExp->body);
  } else if (ast->tag == list_exp || ast->tag == if_exp ||
             ast->tag == global_exp) {
    for (int i_exp = 0; i_exp < get_n_children(ast); ++i_exp) {
      make_scopes(get_child(ast, i_exp));
    }
  } else {
    // Const or var, so don't need to do anything
  }
}

/**
 * Returns:
 *  Pointer to the AST node that has the symbol table
 *  in which ast's variable var is defined,
 *  or NULL if it is undefined
 */
AST *find_scope(AST *ast, char *var) {
  if (map_in(ast->symbol_table, var)) {
    return ast;
  } else {
    if (ast->parent) {
      return find_scope(ast->parent, var);
    } else {
      if (ast->tag == global_exp &&
          map_in(ast->content.globalExp->standard, var)) {
        return ast;
      } else {
        printf("ERROR! Variable/function %s is undefined\n", var);
        return NULL;
      }
    }
  }
}

/**
 * Returns:
 *  Value associated with var in scope,
 *  or NULL if not in scope. NULL could be the value
 *  associated with var in scope, so if ambiguity is
 *  possible then is_in_scope should also be used
 */
void *get_in_scope(AST *ast, char *var) {
  // TODO Error if var is not in scope
  if (map_in(ast->symbol_table, var)) {
    void *value = map_get(ast->symbol_table, var);
    return value;
  } else {
    if (ast->parent != NULL) {
      return get_in_scope(ast->parent, var);
    } else {
      return NULL;
    }
  }
}