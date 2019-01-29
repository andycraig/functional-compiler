#include "ast.h"
#include "scope.h"
#ifndef CLOSURE_CONVERSION_H
#define CLOSURE_CONVERSION_H

int closure_convert(AST *global);

Map *find_free_and_recursive_vars(AST *lambda, AST *global);

#endif