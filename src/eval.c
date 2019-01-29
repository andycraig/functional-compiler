#include "eval.h"
#include <stdio.h>
#include "ast.h"
#include "scope.h"

/**
 * Generates linear code from AST.
 * Returns:
 *   Offset
 */
int eval(FILE *fp, AST *ast, int nth_if, int offset) {
  switch (ast->tag) {
    case lambda_exp: {
      int memory_reqd = get_memory_reqd_by_fn(ast);
      emit_fn_head(fp, ast->content.lambdaExp->name,
                   ast->content.lambdaExp->args, memory_reqd);
      int arg_offset = 0;
      for (int i_arg = 0; i_arg < ast->content.lambdaExp->args->list->len;
           ++i_arg) {
        arg_offset += 8;  // Assumes all args are 8 bytes
        int *i_arg_ptr = malloc(sizeof(*i_arg_ptr));
        *i_arg_ptr = arg_offset;
        map_insert_value(ast->content.lambdaExp->body->symbol_table,
                         get_key_i(ast->content.lambdaExp->args, i_arg),
                         i_arg_ptr);
      }
      offset = eval(fp, ast->content.lambdaExp->body, nth_if, arg_offset);
      emit_fn_tail(fp);
      break;
    }
    case if_exp:
      offset = eval(fp, ast->content.ifExp->pred, nth_if + 1, offset);
      emit_if_pred(fp, nth_if);
      offset = eval(fp, ast->content.ifExp->case_true, nth_if + 1, offset);
      emit_if_true(fp, nth_if);
      offset = eval(fp, ast->content.ifExp->case_false, nth_if + 1, offset);
      emit_if_false(fp, nth_if);
      break;
    case let_exp:
      offset = eval(fp, ast->content.letExp->defn, nth_if, offset);
      offset += 8;  // Assumes let arg is always 8 bytes
      int *arg_ptr = malloc(sizeof(*arg_ptr));
      *arg_ptr = offset;
      map_insert_value(ast->content.letExp->body->symbol_table,
                       ast->content.letExp->arg, arg_ptr);
      emit_let(fp, offset, ast->content.letExp->arg);
      offset = eval(fp, ast->content.letExp->body, nth_if, offset);
      break;
    case list_exp: {  // Function call
      // For example: (f 1 (add 2 3))
      // For example: ((g 4) 1 (add 2 3))
      // For example: (f x y)
      int n_operands = ast->content.listExp->rest->len;
      int *offsets = malloc((n_operands + 1) *
                            sizeof(*offsets));  // +1 for the call pointer
      // Eval operands, contained in ast->content.listExp->rest
      for (int i_operand = n_operands - 1; i_operand >= 0; --i_operand) {
        AST *child = (AST *)get_i(ast->content.listExp->rest, i_operand);
        offset = eval(fp, child, nth_if, offset);
        offset += 8;  // Assuming that all operands are 8 bytes
        offsets[i_operand + 1] = offset;
        emit_operand(fp, offset, i_operand, n_operands);
      }
      if (ast->content.listExp->first->tag == var_exp &&
          ast->content.listExp->first->content.varExp->is_recursive) {
        offset += 8;  // Location for heap-allocated 8-byte arg pointer
        emit_recursive_call(fp, n_operands, offsets,
                            ast->content.listExp->first->content.varExp->name,
                            offset);
      } else {
        // First is function, so eval then call
        eval(fp, ast->content.listExp->first, nth_if, offset);
        offset += 8;  // Call location is 8-byte pointer
        offsets[0] = offset;
        emit_call(fp, n_operands, offsets, offset);
      }
      free(offsets);
      break;
    }
    case var_exp: {
      // For example: x
      AST *scope_node = find_scope(ast, ast->content.varExp->name);
      if (scope_node == NULL) {
        printf("ERROR! Undefined symbol: %s.\n", ast->content.varExp->name);
      }
      void *val = get_in_scope(ast, ast->content.varExp->name);
      if (val == NULL) {
        emit_fn_name(fp, ast->content.varExp->name);
      } else {
        emit_var(fp, *(int *)val, ast->content.varExp->name);
      }
      break;
    }
    case integer_exp:
      // For example: 1
      emit_integer(fp, ast->content.integerExp);
      break;
    case global_exp: {
      emit_global_head(fp);
      for (int i_exp = 0; i_exp < ast->content.globalExp->rest->len; ++i_exp) {
        eval(fp, get_i(ast->content.globalExp->rest, i_exp), nth_if, 0);
      }
      int memory_reqd = get_memory_reqd_by_fn(ast);
      emit_main_head(fp, memory_reqd);
      offset = eval(fp, ast->content.globalExp->main, nth_if, offset);
      emit_main_tail(fp);
      break;
    }
    case make_closure_exp: {
      int *offsets = malloc(ast->content.makeClosureExp->n_free_vars *
                            sizeof(*offsets));  // +1 for the call pointer
      for (int i_free = ast->content.makeClosureExp->n_free_vars - 1;
           i_free >= 0; --i_free) {
        AST *child =
            (AST *)get_i(ast->content.makeClosureExp->free_vars, i_free);
        offset = eval(fp, child, nth_if, offset);
        offset += 8;  // Assuming that all operands are 8 bytes
        offsets[i_free] = offset;
        emit_operand(fp, offset, i_free,
                     ast->content.makeClosureExp->n_free_vars);
      }
      emit_make_closure(fp, ast->content.makeClosureExp->name,
                        ast->content.makeClosureExp->n_bound_vars,
                        ast->content.makeClosureExp->n_free_vars, offsets);
      break;
    }
    default:
      fprintf(fp, "ERROR! Unexpected exp type in eval.\n");
      JSONify_AST(ast);
      break;
  }
  return offset;
}

int get_memory_reqd_by_fn(AST *ast) {
  switch (ast->tag) {
    case integer_exp:
      return 0;
      break;
    case var_exp:
      return 0;
      break;
    case if_exp:
      return get_memory_reqd_by_fn(ast->content.ifExp->pred) +
             get_memory_reqd_by_fn(ast->content.ifExp->case_true) +
             get_memory_reqd_by_fn(ast->content.ifExp->case_false);
      break;
    case lambda_exp:
      return 8 * ast->content.lambdaExp->args->list->len +
             get_memory_reqd_by_fn(ast->content.lambdaExp->body);
      break;
    case let_exp:
      return 8  // For arg
             + get_memory_reqd_by_fn(ast->content.letExp->defn) +
             get_memory_reqd_by_fn(ast->content.letExp->body);
      break;
    case list_exp: {
      int result = 8;  // For result of list evaluation
      for (int i_exp = 0; i_exp < get_n_children(ast); ++i_exp) {
        result += 8;  // For result of operand
        result += get_memory_reqd_by_fn(get_child(ast, i_exp));
      }
      return result;
      break;
    }
    case global_exp:
      return get_memory_reqd_by_fn(ast->content.globalExp->main);
      break;
    case make_closure_exp:
      return 8 * ast->content.makeClosureExp->free_vars->len;
      break;
    default:
      printf("ERROR! Unexpected tag in get_memory_reqd_by_fn.\n");
      printf("Tag: %d\n", ast->tag);
      break;
  }
}

void emit_make_closure(FILE *fp, char *name, int n_bound_vars, int n_free_vars,
                       int *offsets) {
  if (n_free_vars >= 3) {
    fprintf(fp, "\tmov r9, QWORD [rbp-%d]    ; free variable 3/%d\n",
            offsets[2], n_free_vars);
  }
  if (n_free_vars >= 2) {
    fprintf(fp, "\tmov r8, QWORD [rbp-%d]    ; free variable 2/%d\n",
            offsets[1], n_free_vars);
  }
  if (n_free_vars >= 1) {
    fprintf(fp, "\tmov rcx, QWORD [rbp-%d]    ; free variable 1/%d\n",
            offsets[0], n_free_vars);
  }
  fprintf(fp, "\tmov rdx, %d            ; number of free variables\n",
          n_free_vars);
  fprintf(fp, "\tmov rsi, %d            ; number of bound variables\n",
          n_bound_vars);
  fprintf(fp, "\tmov rdi, %s            ; name of function\n", name);
  fprintf(fp, "\tcall make_closure\n");
}

void emit_global_head(FILE *fp) {
  fprintf(fp, "\tglobal main\n");
  fprintf(fp, "\textern printf, malloc                ; C functions\n");
  fprintf(fp, "\textern make_closure, call_closure    ; built-in functions\n");
  fprintf(fp,
          "\textern plus, minus, equals           ; standard "
          "library functions\n");
  fprintf(fp, "\n");
  fprintf(fp, "\tsection .text\n");
}

void emit_main_head(FILE *fp, int memory_reqd) {
  fprintf(fp, "main:\n");
  fprintf(fp, "\tpush rbp\n");
  fprintf(fp, "\tmov rbp, rsp\n");
  fprintf(fp, "\tsub rsp, %d        ; memory for local variables\n",
          memory_reqd);
}

void emit_main_tail(FILE *fp) {
  fprintf(fp, "\tmov rsi, rax        ; will print rax\n");
  fprintf(fp, "\tmov rdi, message\n");
  fprintf(fp, "\tmov rax, 0\n");
  fprintf(fp, "\tcall printf\n");
  fprintf(fp, "\tmov rax, 0            ; exit code 0\n");
  fprintf(fp, "\tleave\n");
  fprintf(fp, "\tret\n");
  fprintf(fp, "\n");
  fprintf(fp, "\tsection .data\n");
  fprintf(fp,
          "message: db \"%%d\", 10, 0        ; 10 is newline, 0 is "
          "end-of-string\n");
}

void emit_let(FILE *fp, int nth, char *arg) {
  fprintf(fp, "\tmov QWORD [rbp-%d], rax    ; let %s\n", nth, arg);
}

void emit_fn_name(FILE *fp, char *var) {
  fprintf(fp, "\tmov rax, %s            ; access %s\n", var, var);
}

void emit_var(FILE *fp, int nth, char *var) {
  fprintf(fp, "\tmov rax, QWORD [rbp-%d]    ; access %s\n", nth, var);
}

void emit_integer(FILE *fp, int x) {
  fprintf(fp, "\tmov rax, %d                ; integer constant\n", x);
}

/**
 * Params:
 *   nth_operand: 0-indexed
 */
void emit_operand(FILE *fp, int offset, int nth_operand, int n_operands) {
  // Save evaluated operand in memory
  fprintf(fp, "\tmov QWORD [rbp-%d], rax    ; preparing operand %d/%d\n",
          offset, nth_operand + 1, n_operands);
}

void emit_call(FILE *fp, int n_args, int *offsets, int offset) {
  fprintf(fp, "\tmov QWORD [rbp-%d], rax    ; preparing closure\n", offset);
  // Put required number of args into arg registers
  if (n_args >= 4) {
    fprintf(fp, "\tmov r8, QWORD [rbp-%d]    ; operand 4/%d\n", offsets[4],
            n_args);
  }
  if (n_args >= 3) {
    fprintf(fp, "\tmov rcx, QWORD [rbp-%d]    ; operand 3/%d\n", offsets[3],
            n_args);
  }
  if (n_args >= 2) {
    fprintf(fp, "\tmov rdx, QWORD [rbp-%d]    ; operand 2/%d\n", offsets[2],
            n_args);
  }
  if (n_args >= 1) {
    fprintf(fp, "\tmov rsi, QWORD [rbp-%d]    ; operand 1/%d\n", offsets[1],
            n_args);
  }
  // Put closure address in first arg register
  fprintf(fp, "\tmov rdi, QWORD [rbp-%d]    ; operator location\n", offsets[0]);
  fprintf(fp, "\tcall call_closure            ; output goes to rax\n");
}

void emit_recursive_call(FILE *fp, int n_args, int *offsets, char *name,
                         int malloc_offset) {
  // Put required number of args onto the heap
  // TODO Make memory required a parameter
  fprintf(fp, "\tmov rdi, %d        ; memory for recursive call variables\n",
          n_args * 8);
  fprintf(fp, "\tcall malloc             ; memory location goes to rax\n");
  fprintf(fp, "\tmov QWORD [rbp-%d], rax\n", malloc_offset);
  for (int i_arg = 0; i_arg < n_args; ++i_arg) {
    // offsets[0] is reserved for function name, so it's skipped
    fprintf(
        fp,
        "\tmov rbx, QWORD [rbp-%d]    ; operand %d/%d for recursive call...\n",
        offsets[i_arg + 1], i_arg + 1, n_args);
    fprintf(fp, "\tmov QWORD [rax+%d], rbx    ; ...loaded onto heap\n",
            i_arg * 8);
  }
  fprintf(
      fp,
      "\tmov rdi, QWORD [rbp-%d]        ; pointer to args for recursive call\n",
      malloc_offset);
  fprintf(fp, "\tcall %s            ; output goes to rax\n", name);
  // TODO Free malloc'd memory but moving rax to stack, moving malloc result to
  // rax,
  // TODO freeing memory, then moving call result back to rax
}

void emit_if_pred(FILE *fp, int nth_if) {
  fprintf(fp, "\tcmp rax, 0\n");
  fprintf(fp, "\tje .L%dFalse\n", nth_if);
}

void emit_if_true(FILE *fp, int nth_if) {
  fprintf(fp, "\tjmp .L%dDone\n", nth_if);
  fprintf(fp, ".L%dFalse:\n", nth_if);
}

void emit_if_false(FILE *fp, int nth_if) { fprintf(fp, ".L%dDone:\n", nth_if); }

void emit_fn_head(FILE *fp, char *name, Map *args, int memory_reqd) {
  fprintf(fp, "%s:\n", name);
  fprintf(fp, "\tpush rbp\n");
  fprintf(fp, "\tmov rbp, rsp\n");
  fprintf(fp, "\tsub rsp, %d        ; memory for local variables\n",
          memory_reqd);
  fprintf(fp, "\tmov rax, rdi       ; pointer to vector of arguments\n");
  for (int i_arg = 1; i_arg <= args->list->len; ++i_arg) {
    fprintf(fp, "\tmov rbx, QWORD [rax+%d]    ; move %s from heap\n",
            (i_arg - 1) * 8, (char *)get_key_i(args, i_arg - 1));
    fprintf(fp, "\tmov QWORD [rbp-%d], rbx    ; move %s to stack\n", i_arg * 8,
            (char *)get_key_i(args, i_arg - 1));
  }
}

int emit_fn_tail(FILE *fp) {
  fprintf(fp, "\tleave\n");
  fprintf(fp, "\tret\n");
}