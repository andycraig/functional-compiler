#include "tokenise.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "ll.h"

void add_token(LL *tokens, char *symbol, int line, int start_char) {
  Token *token = malloc(sizeof(*token));
  token->name = malloc((strlen(symbol) + 1) * sizeof(*token->name));
  strcpy(token->name, symbol);
  token->line = line;
  token->start_char = start_char;
  token->end_char = start_char + strlen(symbol) - 1;
  push_tail(tokens, token);
}

void clear_symbol(char *s) {
  for (int i_char = 0; i_char < SYMBOL_MAX_LENGTH; ++i_char) {
    s[i_char] = '\0';
  }
}

char get_next_char(FILE *fp, int *pos_char) {
  (*pos_char)++;
  return fgetc(fp);
}

/**
 * Reads in a code file and creates a linked list of tokens.
 * Example: With code file content:
 * (define x 1)
 * list will have elements:
 * '(', 'define', 'x', '1', ')'
 */
int tokenise(FILE *fp, LL *list) {
  short in_comment = 0;
  int ch;
  char symbol[SYMBOL_MAX_LENGTH + 1];  // +1 to allow for trailing \0
  clear_symbol(symbol);
  int pos_char = -1;
  int start_char = -1;
  int pos_line = 0;
  tokenise_state = IN_WHITESPACE;  // tokenise_state defined in header
  while ((ch = get_next_char(fp, &pos_char)) != EOF) {
    if (tokenise_state == IN_COMMENT) {
      if (ch == '\n') {
        pos_line++;
        tokenise_state = IN_WHITESPACE;
      }
    } else {
      switch (ch) {
        case '(': {
          add_token(list, "(", pos_line, pos_char);
          break;
        }
        case ')': {
          if (tokenise_state == IN_SYMBOL) {
            // likethis), so handle symbol first
            add_token(list, symbol, pos_line, start_char);
            tokenise_state = IN_WHITESPACE;
          }
          add_token(list, ")", pos_line, pos_char);
          break;
        }
        case ';':
          if (tokenise_state == IN_SYMBOL) {
            // likethis;, so handle symbol first
            add_token(list, ")", pos_line, pos_char);
          }
          tokenise_state = IN_COMMENT;
          break;
        case '\n':
          pos_line++;
          // Intentional fall-through
        case ' ':
        case '\t':
          if (tokenise_state == IN_SYMBOL) {
            add_token(list, symbol, pos_line, start_char);
          }
          tokenise_state = IN_WHITESPACE;
          break;
        default:  // Any non-whitespace, non-parenthesis character
          if (tokenise_state == IN_WHITESPACE) {
            tokenise_state = IN_SYMBOL;
            start_char = pos_char;
          }
          if (pos_char - start_char < SYMBOL_MAX_LENGTH) {
            symbol[pos_char - start_char] = ch;
            symbol[pos_char - start_char + 1] = '\0';
          } else {
            printf("ERROR! Symbol exceed max length of %d.\n",
                   SYMBOL_MAX_LENGTH);
            return TOKENISE_ERROR;
          }
          break;
      }
    }
  }
  return 0;
}

void free_tokens(LL *list) {
  for (int i_token = 0; i_token < list->len; ++i_token) {
    Token *t = get_i(list, i_token);
    free(t->name);
    free(t);
  }
  free_list(list);
}