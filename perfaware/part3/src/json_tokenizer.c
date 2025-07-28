#include "json_tokenizer.h"
#include "global.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int isString(char c) { return isalnum(c); }
int isNumber(char c) { return isnumber(c); }

FILE *source;
char *buffer;

// #define SOURCE_END (-1)
// char getChar() { return getc(source); }
// void consume() { getChar(); }
// void ungetChar(char c) { ungetc(c, source); }

// void initTokenizer(char *filename) { source = fopen(filename, "rb"); }

/* faster to read from buffer */
#define SOURCE_END ('\0')
char getChar() { return *buffer++; }
void consume() { getChar(); }
void ungetChar(char c) { buffer -= 1; }

void initTokenizer(char *filename) { buffer = read_file(filename); }

char *toStringToken(Token token) {
  switch (token) {
  case T_NONE:
    return "T_NONE";
  case T_LEFT_BRACE:
    return "T_LEFT_BRACE";
  case T_RIGHT_BRACE:
    return "T_RIGHT_BRACE";
  case T_QUOTE:
    return "T_QUOTE";
  case T_STRING:
    return "T_STRING";
  case T_COLON:
    return "T_COLON";
  case T_COMMA:
    return "T_COMMA";
  case T_LEFT_BRACKET:
    return "T_LEFT_BRACKET";
  case T_RIGHT_BRACKET:
    return "T_RIGHT_BRACKET";
  case T_NUMBER:
    return "T_NUMBER";
  default:
    return "T_ERROR";
  }
}

int tryMatch(Token token) {
  Token t = nextToken();
  fprintf(verboseChannel, "tryMatch -> token: %s, t: %s\n",
          toStringToken(token), toStringToken(t));

  if (t == token) {
    // if it matched we do consume the token
    consume();
    return 1;
  } else {
    fprintf(verboseChannel, "expected %s but got %s\n", toStringToken(token),
            toStringToken(t));
    return 0;
  }
}

void match(Token token) {
  Token t = nextToken();
  fprintf(verboseChannel, "match -> token: %s, t: %s\n", toStringToken(token),
          toStringToken(t));

  if (t == token) {
    // if it matched we do consume the token
    consume();
  } else {
    fprintf(stderr, "expected %s but got %s\n", toStringToken(token),
            toStringToken(t));
    exit(1);
  }
}

/* advances through the stream until we hit a token */
Token nextToken() {
  fprintf(verboseChannel, "nextToken\n");
  char c;
  Token result = T_ERROR;

  while ((c = getChar()) != SOURCE_END) {
    fprintf(verboseChannel, "considering %c\n", c);

    if (c == ' ' || c == '\n') {
      continue;
    }

    switch (c) {
    case '{': {
      result = T_LEFT_BRACE;
      break;
    }
    case '}': {
      result = T_RIGHT_BRACE;
      break;
    }
    case '"': {
      result = T_QUOTE;
      break;
    }
    case ':': {
      result = T_COLON;
      break;
    }
    case '[': {
      result = T_LEFT_BRACKET;
      break;
    }
    case ']': {
      result = T_RIGHT_BRACKET;
      break;
    }
    case '0' ... '9':
    case '-': {
      fprintf(verboseChannel, "T_NUMBER\n");

      int i = 0;

      if (c == '-') {
        currentString[i++] = c;
      } else {
        ungetChar(c);
      }

      while (isNumber(c = getChar())) {
        fprintf(verboseChannel, "  -> considering %c\n", c);
        currentString[i++] = c;
      }

      if (c == '.') {
        currentString[i++] = c;

        // after the dot
        while (isNumber(c = getChar())) {
          fprintf(verboseChannel, "  -> considering %c\n", c);
          currentString[i++] = c;
        }
        fprintf(verboseChannel, "  -> ends with %c\n", c);
      }

      currentString[i] = '\0';

      // there is not terminator to put back, in this case
      // ungetChar(c);

      result = T_NUMBER;
      break;
    }
    case 'a' ... 'z':
    case 'A' ... 'Z': {
      fprintf(verboseChannel, "T_STRING\n");
      // put the char back to simplify the loop
      ungetChar(c);
      int i = 0;
      while (isString(c = getChar())) {
        fprintf(verboseChannel, "  -> considering %c\n", c);
        currentString[i++] = c;
      }
      fprintf(verboseChannel, "  -> ends with %c\n", c);

      currentString[i] = '\0';

      // we have no quote to put back, unlike
      // in string below...
      // so that ungetChar below will put the
      // end of the string back
      // so that when the caller "consumes"
      // they will be consuming the end of the string
      ungetChar(c);

      result = T_STRING;
      break;
    }
    default: {
      result = T_ERROR;
      break;
    }
    }

    if (result != T_ERROR) {
      // we always unget because it is the job of "consume" to consume the
      // tokens
      fprintf(verboseChannel, "  -> putting %c back\n", c);
      ungetChar(c);
      fprintf(verboseChannel, "got %s\n", toStringToken(result));

      break;
    }
  }

  return result;
}
