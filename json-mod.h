#ifndef __JSON_MOD_H__
#define __JSON_MOD_H__

#include <glib-object.h>
#include <json-glib/json-glib.h>

const char* jm_object_get_string_member(JsonObject* obj, int length, ...);
gboolean jm_object_set_string_member(JsonObject* obj, const char*, int length, ...);
int jm_object_get_int_member(JsonObject* obj, int length, ...);
/**
 This api will make the new JsonNode like clone
 Even if @node has immutable property, returned value will not have immutable.

 @param node the pointer that is used for source
 @return the pointer of JsonNode that has cloned
 @remark You should release returned value using json_node_unref()
  */
JsonNode* jm_node_clone(JsonNode* node);

#endif
