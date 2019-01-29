#include <stdio.h>
#include "ast.h"

#ifndef EVAL_H
#define EVAL_H

int eval(FILE *fp, AST *ast, int nth_if, int offset);

int get_memory_reqd_by_fn(AST *ast);

void emit_make_closure(FILE *fp, char *name, int n_bound_vars, int n_free_vars,
                       int *offsets);

void emit_global_head(FILE *fp);

void emit_main_head(FILE *fp, int memory_reqd);

void emit_main_tail(FILE *fp);

void emit_let(FILE *fp, int nth, char *arg);

void emit_fn_name(FILE *fp, char *name);

void emit_var(FILE *fp, int nth, char *var);

void emit_integer(FILE *fp, int x);

void emit_operand(FILE *fp, int offset, int nth_operand, int n_operands);

void emit_call(FILE *fp, int n_args, int *arg_offsets, int offset);

void emit_recursive_call(FILE *fp, int n_args, int *offsets, char *name,
                         int malloc_offset);

void emit_if_pred(FILE *fp, int nth_if);

void emit_if_true(FILE *fp, int nth_if);

void emit_if_false(FILE *fp, int nth_if);

void emit_fn_head(FILE *fp, char *name, Map *args, int memory_reqd);

int emit_fn_tail(FILE *fp);

#endif