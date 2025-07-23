#include "json_parser.h"
#include "global.h"
#include "json_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void emitter(char *str) {
  if (emit) {
    printf("%s", str);
  }
}

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
  keyValuePairs->type = JSON_PAIR;
  keyValuePairs->next = 0;

  JSONNode *firstElement = keyValuePairs;

  while (value(keyValuePairs)) {
    // we only assigne keyValuePairs to node->value
    // if there are elements. That way an empty array
    // has node->value == null, which we can test
    node->value = firstElement;

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
  TimeFunction;

  JSONNode *root = (JSONNode *)malloc(sizeof(JSONNode));
  memset(root, 0, sizeof(JSONNode));

  object(root);

  return root;
}
