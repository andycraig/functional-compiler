#include "ast.h"
#ifndef SCOPE_H
#define SCOPE_H

void make_scopes(AST *ast);

AST *find_scope(AST *ast, char *var);

void *get_in_scope(AST *ast, char *var);

#endif