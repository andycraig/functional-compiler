#include "closure_conversion.h"
#include <string.h>
#include "ast.h"
#include "global.h"
#include "scope.h"

/**
 * Takes an AST containing lambdas and performs the following:
 * -Lifts lambda declarations to global functions with free parameters converted
 *  to bound variables, and assigns them a name.
 * -Replaces them in the AST with make_closure calls to the now-global
 * functions.
 *
 * Params:
 *  global: The top-level AST node
 */
int closure_convert(AST *global) {
  // Traverse AST post-order, so first build queue
  LL *stack = get_post_order_ast(global);
  int nth_closure = 0;
  while (stack->len > 0) {
    AST *current = (AST *)pop_head(stack);
    if (current->tag == lambda_exp) {
      // Convert this closure
      // Generate unique name for fn
      // TODO Make length of memory for temp_name non-arbitrary
      char *temp_name = (char *)malloc(sizeof(char) * 16);
      sprintf(temp_name, "_f%d", nth_closure);
      current->content.lambdaExp->name = temp_name;
      // Extend lambda's args to include free variables
      int n_bound_vars = current->content.lambdaExp->args->list->len;
      Map *free_vars = find_free_and_recursive_vars(current, global);
      if (free_vars == NULL) {
        free_list(stack);
        return SCOPE_ERROR;
      }
      int n_free_vars = free_vars->list->len;
      for (int i_var = 0; i_var < n_free_vars; ++i_var) {
        map_insert_key(current->content.lambdaExp->args,
                       get_key_i(free_vars, i_var));
        map_insert_key(current->content.lambdaExp->body->symbol_table,
                       get_key_i(free_vars, i_var));
        // TODO Free arg memory
      }
      // Look for recursive calls in lambda body, and add in free variables
      LL *body_stack = make_list();
      push_head(body_stack, current->content.lambdaExp->body);
      while (body_stack->len > 0) {
        AST *body_current = (AST *)pop_head(body_stack);
        if (body_current->tag == list_exp) {
          if (body_current->content.listExp->first->tag == var_exp) {
            // TODO Verify that varExp is recursive with current
            if (body_current->content.listExp->first->content.varExp
                    ->is_recursive) {
              for (int i_free = 0; i_free < n_free_vars; ++i_free) {
                AST *free_node = make_varExp(get_key_i(free_vars, i_free));
                push_tail(body_current->content.listExp->rest, free_node);
                free_node->parent = body_current;
              }
            }
          }
        }
        for (int i_child = 0; i_child < get_n_children(body_current);
             ++i_child) {
          push_head(body_stack, get_child(body_current, i_child));
        }
      }
      free_list(body_stack);
      // Move lambda node to top
      AST *new_node = make_lambdaExp(current->content.lambdaExp->args,
                                     current->content.lambdaExp->body);
      new_node->content.lambdaExp->name = temp_name;
      new_node->content.lambdaExp->body->parent = new_node;
      push_tail(global->content.globalExp->rest, new_node);
      new_node->parent = global;
      // Construct make_closure_exp replacement for lambda in AST:
      current->tag = make_closure_exp;
      current->content.makeClosureExp = malloc(sizeof(SMakeClosureExp));
      // Add function name
      current->content.makeClosureExp->name = temp_name;
      current->content.makeClosureExp->n_bound_vars = n_bound_vars;
      current->content.makeClosureExp->n_free_vars = n_free_vars;
      current->content.makeClosureExp->free_vars = make_list();
      // Add free variables
      for (int i_free = 0; i_free < n_free_vars; ++i_free) {
        AST *new_var_node = make_varExp(get_key_i(free_vars, i_free));
        push_tail(current->content.makeClosureExp->free_vars, new_var_node);
        new_var_node->parent = current;
      }
      ++nth_closure;
    }
  }
  free_list(stack);
  return 0;
}

Map *find_free_and_recursive_vars(AST *lambda, AST *global) {
  // Find all the variables in the lambda body that are not defined in any scope
  // at or below the lambda, and not in the lambda's args. Because lambda body
  // can itself contains lambdas and lets, variable names can be inconsistent,
  // so need to check the scope each time a variable is encountered.
  Map *free_vars = make_map(str_eq);
  LL *stack = make_list();
  push_head(stack, lambda->content.lambdaExp->body);
  while (stack->len > 0) {
    AST *current = pop_head(stack);
    if (current->tag == var_exp) {
      char *var = current->content.varExp->name;
      int is_var_bound = map_in(lambda->content.lambdaExp->args, var);
      if (!is_var_bound) {
        AST *scope_source = find_scope(current, var);
        if (scope_source == NULL) {
          printf("ERROR! Variable %s is undefined\n", var);
          return NULL;
        } else {
          char *scope_value = (char *)get_in_scope(current, var);
          if (scope_value != NULL && strcmp(scope_value, "RECURSIVE") == 0) {
            current->content.varExp->name = lambda->content.lambdaExp->name;
            current->content.varExp->is_recursive = 1;
          } else {
            if (is_ancestor(scope_source, lambda)) {
              map_insert_key(free_vars, current->content.varExp->name);
            } else {
              // Var was defined inside lambda, so it's not free
            }
          }
        }
      }
    } else if (get_n_children(current) > 0) {
      // Push children to stack.
      for (int i_exp = 0; i_exp < get_n_children(current); ++i_exp) {
        push_head(stack, get_child(current, i_exp));
      }
    } else {
      // Const exp. Do nothing.
    }
  }
  free_list(stack);
  return free_vars;
}