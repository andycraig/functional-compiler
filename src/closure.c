#include "closure.h"
#include <stdlib.h>

FirstClass *make_data(long val) {
  FirstClass *fc = malloc(sizeof(*fc));
  fc->tag = data_tag;
  fc->val.data = val;
  return fc;
}

FirstClass *make_closure(long (*codeptr)(long *), long n_bound_vars,
                         long n_free_vars, long freevar1, long freevar2,
                         long freevar3) {
  FirstClass *cl = malloc(sizeof(*cl));
  cl->tag = closure_tag;
  cl->val.closure.codeptr = codeptr;
  cl->val.closure.n_bound_vars = n_bound_vars;
  cl->val.closure.n_free_vars = n_free_vars;
  cl->val.closure.freevar =
      malloc(n_free_vars * sizeof(*cl->val.closure.freevar));
  if (cl->val.closure.n_free_vars >= 1) {
    cl->val.closure.freevar[0] = freevar1;
  }
  if (cl->val.closure.n_free_vars >= 2) {
    cl->val.closure.freevar[1] = freevar2;
  }
  if (cl->val.closure.n_free_vars >= 3) {
    cl->val.closure.freevar[2] = freevar3;
  }
  return cl;
}

FirstClass *call_closure(FirstClass *cl, long boundvar1, long boundvar2,
                         long boundvar3, long boundvar4) {
  // TODO Verify that cl is indeed a closure
  int n_vars = cl->val.closure.n_bound_vars + cl->val.closure.n_free_vars;
  long *var = malloc(n_vars * sizeof(*var));
  // Fill bound vars from those provided when closure was CALLED
  if (cl->val.closure.n_bound_vars >= 1) {
    var[0] = boundvar1;
  }
  if (cl->val.closure.n_bound_vars >= 2) {
    var[1] = boundvar2;
  }
  if (cl->val.closure.n_bound_vars >= 3) {
    var[2] = boundvar3;
  }
  if (cl->val.closure.n_bound_vars >= 4) {
    var[3] = boundvar4;
  }
  // Fill free vars from those provided when closure was CREATED
  for (int i_free = 0; i_free < cl->val.closure.n_free_vars; ++i_free) {
    // When compiled with gcc -S closure.c
    // the next line is between .L11 and .L10
    var[i_free + cl->val.closure.n_bound_vars] =
        cl->val.closure.freevar[i_free];
  }
  FirstClass *result;
  result = (FirstClass *)cl->val.closure.codeptr(var);
  return result;
}
