#include "ast.h"
#include "ll.h"

#ifndef PARSE_H
#define PARSE_H

int parse(LL *tokens, AST *global);

int process_defines(AST *global);

int parse_special_forms(AST *ast);

void join_parents(AST *ast);

#endif