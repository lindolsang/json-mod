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

/**
 * @brief This api will return the value that is related about passed node path
 * @param[in] obj The pointer of the json_object in top level
 * @param[in] node_path the path string that you need to get
 * @return string value of the path that you passed with parameter.
 * if can not be found the value with the node_path, NULL will be returned.
 */
const gchar* jm_object_dot_get_string_member(JsonObject* obj, const char* node_path);

/**
 * @brief This api will return the value that is related about passed node path
 * @param[in] obj The pointer of the json_object in top level
 * @param[in] node_path the path string that you need to get
 * @return integer value of the path that you passed with parameter.
 */
int jm_object_dot_get_int_member(JsonObject* obj, const char* node_path);

/**
 * @brief This api return the value that is related about passed node_path
 * @param[in] obj The pointer of the json_object in top level
 * @param[in] node_path the path string that you need to get
 * @return double value of the path that you passed with parameter.
 */
double jm_object_dot_get_double_member(JsonObject* obj, const char* node_path);

/**
 * @brief This api return the value that is related about passed node_path
 * @param[in] obj The pointer of the json_object in top level
 * @param[in] node_path the path string that you need to get
 * @return Boolean value of the path that you passed witt parameter
 * TRUE or FALSE will be returned.
 */
gboolean jm_object_dot_get_boolean_member(JsonObject* obj, const char* node_path);
#endif
