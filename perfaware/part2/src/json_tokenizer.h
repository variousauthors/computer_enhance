#ifndef JSON_TOKENIZER_H
#define JSON_TOKENIZER_H

#include <stdio.h>

char currentString[1024];

FILE *source;

typedef enum Token {
  T_NONE,
  T_LEFT_BRACE,
  T_RIGHT_BRACE,
  T_LEFT_BRACKET,
  T_RIGHT_BRACKET,
  T_QUOTE,
  T_MINUS,
  T_NUMBER,
  T_DOT,
  T_STRING,
  T_COLON,
  T_COMMA,
  T_EOF,
  T_ERROR,
} Token;

Token nextToken();
int tryMatch(Token token);
void match(Token token);

#endif