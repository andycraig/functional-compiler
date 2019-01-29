#include <stdio.h>
#include "ll.h"
#ifndef TOKENISE_H
#define TOKENISE_H

typedef struct Token {
  char *name;
  int line;
  int start_char;
  int end_char;
} Token;

enum TokeniseState { IN_WHITESPACE, IN_COMMENT, IN_SYMBOL } tokenise_state;

void clear_symbol(char *s);

char get_next_char(FILE *fp, int *pos_char);

int tokenise(FILE *fp, LL *list);

void add_token(LL *tokens, char *symbol, int line, int start_char);

void free_tokens(LL *list);

#endif