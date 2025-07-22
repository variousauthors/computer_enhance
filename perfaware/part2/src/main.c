#include <alloca.h>
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

typedef enum JSONNodeType {
  JSON_ERROR,
  JSON_PAIR,
  JSON_STRING,
  JSON_NUMBER,
  JSON_ARRAY,
  JSON_OBJECT,
} JSONNodeType;

char *toStringJSONType(JSONNodeType type) {
  switch (type) {
  case JSON_ERROR:
    return "error";
  case JSON_PAIR:
    return "pair";
  case JSON_ARRAY:
    return "array";
  case JSON_STRING:
    return "string";
  case JSON_OBJECT:
    return "object";
  case JSON_NUMBER:
    return "number";

  default:
    break;
  }
}
typedef struct JSONNode {
  JSONNodeType type;
  struct JSONNode *next;
  struct JSONNode *value;

  char *key;
  union scalar {
    double number;
    char *string;
  } scalar;

} JSONNode;

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
int isNumber(char c) { return isnumber(c); }

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
        ungetc(c, source);
      }

      while (isNumber(c = getc(source))) {
        fprintf(verboseChannel, "  -> considering %c\n", c);
        currentString[i++] = c;
      }

      if (c == '.') {
        currentString[i++] = c;

        // after the dot
        while (isNumber(c = getc(source))) {
          fprintf(verboseChannel, "  -> considering %c\n", c);
          currentString[i++] = c;
        }
        fprintf(verboseChannel, "  -> ends with %c\n", c);
      }

      currentString[i] = '\0';

      // there is not terminator to put back, in this case
      // ungetc(c, source);

      result = T_NUMBER;
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
        currentString[i++] = c;
      }
      fprintf(verboseChannel, "  -> ends with %c\n", c);

      currentString[i] = '\0';

      // we have no quote to put back, unlike
      // in string below...
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

int string();
int array(JSONNode *node);
int number();
int object(JSONNode *node);
int value(JSONNode *node);

int number() {
  fprintf(verboseChannel, "number\n");
  if (!tryMatch(T_NUMBER)) {
    return 0;
  }

  emitter(currentString);

  return 1;
}

int array(JSONNode *node) {
  fprintf(verboseChannel, "array\n");
  if (!tryMatch(T_LEFT_BRACKET)) {
    return 0;
  }
  emitter("[");

  node->type = JSON_ARRAY;

  int i = 0;

  JSONNode *keyValuePairs = (JSONNode *)malloc(sizeof(JSONNode));
  memset(keyValuePairs, 0, sizeof(JSONNode));
  node->value = keyValuePairs;
  keyValuePairs->type = JSON_PAIR;
  keyValuePairs->next = 0;

  while (value(keyValuePairs)) {
    snprintf(currentString, sizeof(currentString), "%d", i);
    keyValuePairs->key = (char *)malloc(sizeof(currentString));
    strcpy(keyValuePairs->key, currentString);

    if (tryMatch(T_RIGHT_BRACKET)) {
      break;
    } else {
      // more elements follow
      emitter(",");
      i++;
      keyValuePairs->next = (JSONNode *)malloc(sizeof(JSONNode));
      memset(keyValuePairs->next, 0, sizeof(JSONNode));

      keyValuePairs = keyValuePairs->next;
      keyValuePairs->next = 0;
      keyValuePairs->type = JSON_PAIR;
    }
  }

  emitter("]");

  return 1;
}

int string() {
  fprintf(verboseChannel, "string\n");
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

int key(JSONNode *node) {
  fprintf(verboseChannel, "key\n");
  int result = string();

  // allocate the key
  node->key = (char *)malloc(sizeof(currentString));
  strcpy(node->key, currentString);

  return result;
}

int value(JSONNode *node) {
  fprintf(verboseChannel, "value\n");
  Token t = nextToken();
  fprintf(verboseChannel, "got %s\n", toStringToken(t));

  JSONNode *valueNode = (JSONNode *)malloc(sizeof(JSONNode));
  memset(valueNode, 0, sizeof(JSONNode));
  node->value = valueNode;

  switch (t) {
  case T_QUOTE: {
    string();

    valueNode->type = JSON_STRING;
    valueNode->scalar.string = (char *)malloc(sizeof(currentString));
    strcpy(valueNode->scalar.string, currentString);

    return 1;
  }
  case T_LEFT_BRACKET: {
    fprintf(verboseChannel, "array\n");
    array(valueNode);

    return 1;
  }
  case T_NUMBER: {
    fprintf(verboseChannel, "number\n");
    emitter(currentString);

    valueNode->type = JSON_NUMBER;
    char *endPtr;
    valueNode->scalar.number = strtod(currentString, &endPtr);

    return 1;
  }
  case T_LEFT_BRACE: {
    fprintf(verboseChannel, "object\n");
    object(valueNode);

    return 1;
  }
  default:
    fprintf(verboseChannel, "no value\n");
    // if we didn't get one of these we should put it back
    return 0;
    break;
  }
}

/** recursively parses a json object from the stream */
int object(JSONNode *node) {
  fprintf(verboseChannel, "object\n");
  if (!tryMatch(T_LEFT_BRACE)) {
    return 0;
  }
  emitter("{");

  node->type = JSON_OBJECT;

  JSONNode *keyValuePairs = (JSONNode *)malloc(sizeof(JSONNode));
  memset(keyValuePairs, 0, sizeof(JSONNode));
  node->value = keyValuePairs;
  keyValuePairs->next = 0;

  int i = 0;
  while (key(keyValuePairs)) {
    fprintf(verboseChannel, "matched key\n");

    keyValuePairs->type = JSON_PAIR;

    match(T_COLON);
    emitter(":");
    value(keyValuePairs);

    if (tryMatch(T_RIGHT_BRACE)) {
      // no more
      break;
    } else {
      // more
      emitter(",");

      keyValuePairs->next = (JSONNode *)malloc(sizeof(JSONNode));
      memset(keyValuePairs->next, 0, sizeof(JSONNode));

      keyValuePairs = keyValuePairs->next;
      keyValuePairs->next = 0;
    }
  }

  emitter("}");

  return 1;
}

JSONNode *parseJSON() {
  JSONNode *root = (JSONNode *)malloc(sizeof(JSONNode));
  memset(root, 0, sizeof(JSONNode));

  object(root);

  return root;
}

/** given an object returns the value node of the node at the given key */
JSONNode *getElementByKey(JSONNode *node, char *str) {
  if (node->type != JSON_OBJECT) {
    return 0;
  }

  // the value node of an OBJECT is a linked list of k/v pairs
  JSONNode *pair = node->value;

  // now we are iterating over keys/value pairs
  while (pair != 0) {
    if (strcmp(pair->key, str) == 0) {
      // return the value node at that key
      pair = pair->value;
      break;
    }

    pair = pair->next;
  }

  return pair;
}

void printObject(JSONNode *object) {
  fprintf(verboseChannel, "type: %s\n", toStringJSONType(object->type));

  // a linked list of pairs
  JSONNode *pair = object->value;

  while (pair != 0) {
    // pair->value is another k/v pair
    fprintf(verboseChannel, "-> type: %s, key: %s\n",
            toStringJSONType(pair->type), pair->key);

    if (pair->value->type == JSON_STRING) {
      fprintf(verboseChannel, "  -> value: { type: %s, value: %s }\n",
              toStringJSONType(pair->value->type), pair->value->scalar.string);
    } else {
      fprintf(verboseChannel, "  -> value: { type: %s, value: %f }\n",
              toStringJSONType(pair->value->type), pair->value->scalar.number);
    }

    fprintf(verboseChannel, "  -> next: %p, value: %p\n", pair->next,
            pair->value);

    pair = pair->next;
  }
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

  JSONNode *root = parseJSON();

  printObject(getElementByKey(root, "letters"));
  printObject(getElementByKey(root, "strings"));
  printObject(getElementByKey(root, "pairs"));
  printObject(getElementByKey(root, "numbers"));
}
