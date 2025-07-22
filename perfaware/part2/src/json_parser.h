#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdio.h>

typedef enum JSONNodeType {
  JSON_ERROR,
  JSON_PAIR,
  JSON_STRING,
  JSON_NUMBER,
  JSON_ARRAY,
  JSON_OBJECT,
} JSONNodeType;

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

JSONNode *parseJSON();
char *toStringJSONType(JSONNodeType type);

#endif