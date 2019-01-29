#ifndef CLOSURE_H
#define CLOSURE_H

typedef struct Closure {
  long (*codeptr)(long *);  // Pointer to function
  long n_bound_vars;
  long n_free_vars;
  long *freevar;
} Closure;

typedef struct FirstClass {
  enum { data_tag, closure_tag } tag;
  union {
    long data;
    Closure closure;
  } val;
} FirstClass;

FirstClass *make_data(long val);

FirstClass *make_closure(long (*codeptr)(long *), long n_bound_vars,
                         long n_free_vars, long freevar1, long freevar2,
                         long freevar3);

FirstClass *call_closure(FirstClass *cl, long boundvar1, long boundvar2,
                         long boundvar3, long boundvar4);

#endif
