#include "parse.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "global.h"
#include "ll.h"
#include "tokenise.h"

/**
 * Takes a list of tokens and creates AST.
 */
int parse(LL *tokens, AST *global) {
  LL *stack = make_list();
  AST *list_start_delim = make_listStart();
  while (tokens->len > 0) {
    Token *token = (Token *)pop_head(tokens);
    if (strcmp(token->name, "(") == 0) {
      push_head(stack, list_start_delim);
    } else if (strcmp(token->name, ")") == 0) {
      // Function complete, so wind back stack until start of function
      AST *node = make_listExp();
      AST *elem2 = (AST *)pop_head(stack);
      AST *elem1 = (AST *)pop_head(stack);
      while (elem1->tag != list_start_token) {
        push_head(node->content.listExp->rest, elem2);
        elem2 = elem1;
        if (stack->len == 0) {
          printf("ERROR! Unmatched ')'.\n");
          return PARSE_ERROR;
        } else {
          elem1 = (AST *)pop_head(stack);
        }
      }
      node->content.listExp->first = elem2;
      if (stack->len == 0) {
        // We have completed a global-level list
        join_parents(node);
        if (tokens->len == 0) {
          // This was the last list in the tokens,
          // which by definition is main
          global->content.globalExp->main = node;
        } else {
          push_tail(global->content.globalExp->rest, node);
        }
        node->parent = global;
      } else {
        push_head(stack, node);
      }
    } else {  // Not a special form
      AST *node;
      if (isdigit(token->name[0]) ||
          (token->name[0] == '-' && isdigit(token->name[1]))) {
        node = make_integerExp(atoi(token->name));
      } else {
        node = make_varExp(token->name);
      }
      push_head(stack, node);
    }
  }
  if (stack->len > 0) {
    printf("ERROR! Unmatched '('.\n");
    return PARSE_ERROR;
  }
  return 0;
}

/**
 * Takes an AST with define expressions and a final
 * 'main' expression, and converts the define
 * expressions to let expressions at the start of the
 * main expression.
 */
int process_defines(AST *global) {
  while (global->content.globalExp->rest->len > 0) {
    AST *def = (AST *)pop_tail(global->content.globalExp->rest);
    char *name = def->content.listExp->first->content.varExp->name;
    if (strcmp(name, "def") == 0 || strcmp(name, "defrec") == 0) {
      // What was main becomes the last element of def,
      // which will later be transformed into the body of a let
      push_tail(def->content.listExp->rest, global->content.globalExp->main);
      global->content.globalExp->main->parent = def;
      global->content.globalExp->main = def;
      def->parent = global;
    } else {
      printf(
          "ERROR! All but last global expression must be 'def' or 'defrec'.");
      return PARSE_ERROR;
    }
  }
  return 0;
}

/**
 * Takes an AST in which special forms
 * are varExps (i.e., just treated as strings),
 * and converts them into specialised AST forms.
 */
int parse_special_forms(AST *ast) {
  if (ast->tag == list_exp) {
    if (ast->content.listExp->first->tag == var_exp) {
      AST *new_exp;
      char *name = ast->content.listExp->first->content.varExp->name;
      if (strcmp(name, "let") == 0 || strcmp(name, "letrec") == 0 ||
          strcmp(name, "def") == 0 || strcmp(name, "defrec") == 0) {
        if (ast->content.listExp->rest->len != 3) {
          printf(
              "ERROR! 'let/letrec/def/defrec' expression must have "
              "argument, definition and body.\n");
          return PARSE_ERROR;
        }
        AST *arg_node = (AST *)get_i(ast->content.listExp->rest, 0);
        if (arg_node->tag != var_exp) {
          printf("ERROR! 'let' expression argument must be a symbol.\n");
          printf("But tag was: %d\n", arg_node->tag);
          return PARSE_ERROR;
        }
        char *arg = arg_node->content.varExp->name;
        AST *defn = (AST *)get_i(ast->content.listExp->rest, 1);
        AST *body = (AST *)get_i(ast->content.listExp->rest, 2);
        int result;
        result = parse_special_forms(defn);
        if (result != 0) {
          return result;
        }
        result = parse_special_forms(body);
        if (result != 0) {
          return result;
        }
        ast->content.letExp = make_letExp(arg, defn, body,
                                          strcmp(name, "letrec") == 0 ||
                                              strcmp(name, "defrec") == 0)
                                  ->content.letExp;
        ast->tag = let_exp;
      } else if (strcmp(name, "lambda") == 0 || strcmp(name, "λ") == 0) {
        // Lambda args are all of rest other than last element, which is body
        if (ast->content.listExp->rest->len == 0) {
          printf("ERROR! 'lambda' expression must have a body.\n");
          return PARSE_ERROR;
        }
        AST *body = (AST *)pop_tail(ast->content.listExp->rest);
        Map *args = make_map(str_eq);
        while (ast->content.listExp->rest->len > 0) {
          AST *exp = (AST *)pop_head(ast->content.listExp->rest);
          if (exp->tag != var_exp) {
            printf(
                "ERROR! All elements of lambda other than last must be "
                "parameter names.\n");
            printf("But tag was: %d\n", exp->tag);
            return PARSE_ERROR;
          }
          char *arg = exp->content.varExp->name;
          map_insert_key(args, arg);
        }
        int result = parse_special_forms(body);
        if (result != 0) {
          return result;
        }
        // TODO Free overwritten listExp?
        ast->content.lambdaExp = make_lambdaExp(args, body)->content.lambdaExp;
        ast->tag = lambda_exp;
      } else if (strcmp(name, "if") == 0) {
        if (ast->content.listExp->rest->len != 3) {
          printf(
              "ERROR! 'if' expression must have predicate, true case and "
              "false case.\n");
          return PARSE_ERROR;
        }
        AST *pred = (AST *)ast->content.listExp->rest->head->val;
        AST *case_true = (AST *)ast->content.listExp->rest->head->next->val;
        AST *case_false =
            (AST *)ast->content.listExp->rest->head->next->next->val;
        int result;
        result = parse_special_forms(pred);
        if (result != 0) {
          return result;
        }
        result = parse_special_forms(case_true);
        if (result != 0) {
          return result;
        }
        result = parse_special_forms(case_false);
        if (result != 0) {
          return result;
        }
        ast->content.ifExp =
            make_ifExp(pred, case_true, case_false)->content.ifExp;
        ast->tag = if_exp;
      } else {
        // Function that is not special form. Leave as-is
      }
    } else {
      // ast is a list_exp, but ast->content.listExp->first is not a var_exp
      if (ast->content.listExp->first->tag == list_exp) {
        for (int i_exp = 0; i_exp < get_n_children(ast); ++i_exp) {
          int result = parse_special_forms(get_child(ast, i_exp));
          if (result != 0) {
            return result;
          }
        }
      }
    }
  } else {
    // ast is not a list_exp. Carry on parsing its children
    if (get_n_children(ast) > 0) {
      for (int i_exp = 0; i_exp < get_n_children(ast); ++i_exp) {
        int result = parse_special_forms(get_child(ast, i_exp));
        if (result != 0) {
          return result;
        }
      }
    }
  }
  return 0;
}

void join_parents(AST *ast) {
  for (int i_child = 0; i_child < get_n_children(ast); ++i_child) {
    AST *this_child = get_child(ast, i_child);
    this_child->parent = ast;
    join_parents(this_child);
  }
}