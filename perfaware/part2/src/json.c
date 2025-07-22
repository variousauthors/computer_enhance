#include "json.h"
#include "global.h"
#include <string.h>

/** given an object returns the value node of the node at the given key */
JSONNode *getValueByKey(JSONNode *node, char *str) {
  if (node->type != JSON_OBJECT && node->type != JSON_ARRAY) {
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
