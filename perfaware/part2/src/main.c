#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int emit;
int verbose;
FILE *verboseChannel = 0;

char currentString[1024];

typedef enum Token {
  T_NONE,
  T_LEFT_BRACE,
  T_RIGHT_BRACE,
  T_QUOTE,
  T_STRING,
  T_COLON,
  T_COMMA,
  T_EOF,
  T_ERROR,
} Token;

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
  default:
    return "T_ERROR";
  }
}

void emitter(char *str) {
  if (emit) {
    printf("%s", str);
  }
}

FILE *source;

char peek() {
  char c = getc(source);
  ungetc(c, source);

  return c;
}

int isString(char c) { return isalnum(c); }

Token toTokenChar(char c) {
  Token result;

  return result;
}

/* advances through the stream until we hit a token */
Token nextToken() {
  fprintf(verboseChannel, "nextToken\n");
  char c;
  Token result = T_ERROR;

  while ((c = getc(source)) != EOF) {
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
    case 'a' ... 'z':
    case 'A' ... 'Z': {
      fprintf(verboseChannel, "T_STRING\n");
      // put the char back to simplify the loop
      ungetc(c, source);
      int i = 0;
      while (isString(c = getc(source))) {
        fprintf(verboseChannel, "  -> considering %c\n", c);
        currentString[i] = c;
        i++;
      }
      fprintf(verboseChannel, "  -> ends with %c\n", c);

      currentString[i] = '\0';

      // we have to put the quote back
      // so that ungetc below will put the
      // end of the string back
      // so that when the caller "consumes"
      // they will be consuming the end of the string
      ungetc(c, source);

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
      ungetc(c, source);

      break;
    }
  }

  return result;
}

void consume() { getc(source); }

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

int string() {
  if (!tryMatch(T_QUOTE)) {
    return 0;
  }
  emitter("\"");

  match(T_STRING);
  emitter(currentString);

  match(T_QUOTE);
  emitter("\"");

  return 1;
}

int key() {
  fprintf(verboseChannel, "key\n");
  return string();
}

void value() {
  // could be a string or an empty array
  Token t = nextToken();

  switch (t) {
  case T_QUOTE: {
    string();
    break;
  }

  default:
    break;
  }
}

/** recursively parses a json object from the stream */
void object() {
  fprintf(verboseChannel, "object\n");
  match(T_LEFT_BRACE);
  emitter("{");

  if (key()) {
    fprintf(verboseChannel, "matched key\n");
    match(T_COLON);
    emitter(":");
    value();
  }

  match(T_RIGHT_BRACE);
  emitter("}");
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0) {
      verbose = 1;
    }

    if (strcmp(argv[i], "-emit") == 0) {
      emit = 1;
    }
  }

  if (verbose) {
    verboseChannel = stderr;
  } else {
    verboseChannel = fopen("/dev/null", "w");
  }

  char *answers = argv[argc - 1];
  char *in = argv[argc - 2];
  source = fopen(in, "rb");
  Token next;

  object();
}
