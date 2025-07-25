#ifndef JSON_H
#define JSON_H

#include "json_parser.h"

JSONNode *getValueByKey(JSONNode *node, char *str);
void printObject(JSONNode *object);

#endif