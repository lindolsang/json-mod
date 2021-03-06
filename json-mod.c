#include "json-mod.h"
#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

int cut_array_index(const char* index_str) {
    char buf[10] = {0, };
    snprintf(buf, 10, "%s", index_str + 1);
    int length = strlen(buf);
    buf[length - 1] = '\0';
    return atoi(buf);
}

/**
  @see https://developer.gnome.org/gobject/stable/gobject-Type-Information.html#GType
  */
static void __gtype_print(GType t)
{
    switch (t) {
        case G_TYPE_INVALID:
            DBG("G_TYPE_INVALID");
            break;
        case G_TYPE_NONE:
            DBG("G_TYPE_NONE");
            break;
        case G_TYPE_INTERFACE:
            DBG("G_TYPE_INTERFACE");
            break;
        case G_TYPE_CHAR:
            DBG("G_TYPE_CHAR");
            break;
        case G_TYPE_UCHAR:
            DBG("G_TYPE_UCHAR");
            break;
        case G_TYPE_BOOLEAN:
            DBG("G_TYPE_BOOLEAN");
            break;
        case G_TYPE_INT:
            DBG("G_TYPE_INT");
            break;
        case G_TYPE_UINT:
            DBG("G_TYPE_UNIT");
            break;
        case G_TYPE_LONG:
            DBG("G_TYPE_LONG");
            break;
        case G_TYPE_ULONG:
            DBG("G_TYPE_ULONG");
            break;
        case G_TYPE_INT64:
            DBG("G_TYPE_INT64");
            break;
        case G_TYPE_UINT64:
            DBG("G_TYPE_UINT64");
            break;
        case G_TYPE_ENUM:
            DBG("G_TYPE_ENUM");
            break;
        case G_TYPE_FLAGS:
            DBG("G_TYPE_FLAGS");
            break;
        case G_TYPE_FLOAT:
            DBG("G_TYPE_FLOAT");
            break;
        case G_TYPE_DOUBLE:
            DBG("G_TYPE_DOUBLE");
            break;
        case G_TYPE_STRING:
            DBG("G_TYPE_STRING");
            break;
        case G_TYPE_POINTER:
            DBG("G_TYPE_POINTER");
            break;
        case G_TYPE_BOXED:
            DBG("G_TYPE_BOXED");
            break;
        case G_TYPE_PARAM:
            DBG("G_TYPE_PARAM");
            break;
        case G_TYPE_OBJECT:
            DBG("G_TYPE_OBJECT");
            break;
        /*case G_TYPE_GTYPE:
            DBG("G_TYPE_GTYPE");
            break;*/
        case G_TYPE_VARIANT:
            DBG("G_TYPE_VARIANT");
            break;
        default:
            DBG("others: %ld", t);
            break;
    }
}

/*
   1. get node type
          if is object
              get members
                  get member
                      go to 1.
          if is array
              get elements
                  get element
                      go to 1.
          if is value
              get type
                  if is int
                      add int value
                  if is double
                      add double value
                  if is boolean
                      add boolean value
                  if is string
                      add string value
                  if is null
                      add null value
*/
static int __jm_json_node_build(JsonNode* node, JsonBuilder* builder)
{
    JsonNodeType node_type = json_node_get_node_type(node);
    switch (node_type) {
        case JSON_NODE_OBJECT:
            {
                JsonObject* node_obj = json_node_get_object(node);
                GList* obj_member_list = json_object_get_members(node_obj);
                GList* l;

                json_builder_begin_object(builder);

                for (l = obj_member_list; l != NULL; l = l->next) {
                    char* member_node_name = (char* )l->data;
                    JsonNode* member_node = json_object_get_member(node_obj, member_node_name);
                    json_builder_set_member_name(builder, member_node_name);

                    __jm_json_node_build(member_node, builder);
                }

                json_builder_end_object(builder);
                g_list_free(obj_member_list);
            }
            break;
        case JSON_NODE_ARRAY:
            {
                JsonArray* node_arr = json_node_get_array(node);
                GList* arr_member_list = json_array_get_elements(node_arr);
                GList* l = NULL;

                json_builder_begin_array(builder);

                for (l = arr_member_list; l != NULL; l = l->next) {
                    JsonNode* member_node = (JsonNode*)l->data;
                    __jm_json_node_build(member_node, builder);
                }

                json_builder_end_array(builder);
                /* what is the free data container? transfer container */
                //TODO needs to check and confirm whether this block has memory leak or not
                //https://developer.gnome.org/glib/stable/glib-Doubly-Linked-Lists.html
                //https://developer.gnome.org/json-glib/stable/json-glib-JSON-Array.html#json-array-get-elements
                g_list_free(arr_member_list);
            }
            break;
        case JSON_NODE_VALUE:
            {
                GType mv_type = json_node_get_value_type(node);
                switch (mv_type) {
                    case G_TYPE_STRING:
                        {
                            json_builder_add_string_value(builder, json_node_get_string(node));
                        }
                        break;
                    case G_TYPE_INT64:
                        {
                            json_builder_add_int_value(builder, json_node_get_int(node));
                        }
                        break;
                    case G_TYPE_DOUBLE:
                        {
                            json_builder_add_double_value(builder, json_node_get_double(node));
                        }
                        break;
                    case G_TYPE_BOOLEAN:
                        {
                            json_builder_add_boolean_value(builder, json_node_get_boolean(node));
                        }
                        break;
                    default:
                        WRN("Unsupported type - %ld", mv_type);
                        // to debug
                        __gtype_print(mv_type);
                        break;
                }
            }
            break;
        case JSON_NODE_NULL:
            {
                WRN("JSON_NODE_NULL");
            }
            break;
    }
    return 0;
}

JsonNode* jm_node_clone(JsonNode* node)
{
    JsonBuilder* builder = json_builder_new();
    __jm_json_node_build(node, builder);
    JsonNode* return_node = json_builder_get_root(builder);
    g_object_unref(builder);

    return return_node;
}

typedef enum {
    NODE_VALUE_TYPE_STRING = 0,
    NODE_VALUE_TYPE_INT,
    NODE_VALUE_TYPE_DOUBLE,
    NODE_VALUE_TYPE_BOOLEAN
} node_value_type_e;

static void* __read_node_value(node_value_type_e read_vt, JsonNode* value_node)
{
    GType value_type = json_node_get_value_type(value_node);
    void *return_v = NULL;
    static int ret_integer = 0;
    static double ret_double = 0.0;
    static gboolean ret_boolean = FALSE;

    switch (value_type) {
        case G_TYPE_STRING:
            return_v = (void *)json_node_get_string(value_node);
            break;
        case G_TYPE_INT64:
            {
                ret_integer = json_node_get_int(value_node);
                return_v = &ret_integer;
            }
            break;
        case G_TYPE_DOUBLE:
            {
                ret_double = json_node_get_double(value_node);
                return_v = &ret_double;
            }
            break;
        case G_TYPE_BOOLEAN:
            {
                ret_boolean = json_node_get_boolean(value_node);
                return_v = &ret_boolean;
            }
            break;
        default:
            // debug
            WRN("Unsupported type: %lu", value_type);
            __gtype_print(value_type);
            break;
    }
    return return_v;
}

static gboolean __chk_has_array_format(const char* str)
{
    if (str == NULL) {
        WRN("str is NULL");
        return FALSE;
    }
    return (strstr(str, "[") ? TRUE : FALSE);
}

static void* __object_get_member_value(node_value_type_e node_vt, JsonObject* obj, const char* node_path)
{
    // parse node name with token
    char* buf_node_path = strdup(node_path);
    char* member_name = NULL;
    JsonObject* member_obj = obj;
    void* return_v = NULL;

    member_name = strtok(buf_node_path, ".");

    if (json_object_has_member(member_obj, member_name) == FALSE) {
        ERR("Invalid member: %s", member_name);
        free(buf_node_path);
        return NULL;
    }
        
    JsonNode* member_node = json_object_get_member(obj, member_name);
    gboolean keep_d = TRUE;
    char buf_member_name[256] = {0, };
    int node_arr_index = 0;
    for (; member_name && keep_d;) {
        //DBG("member_name: %s", member_name);
        JsonNodeType member_type = json_node_get_node_type(member_node);
        switch (member_type) {
            case JSON_NODE_OBJECT:
                //DBG("JSON_NODE_OBJECT");
                {
                    member_obj = json_node_get_object(member_node);
                    member_name = strtok(NULL, ".");
                    if (__chk_has_array_format(member_name)) {
                        //WRN("member_name has array format");
                        strncpy(buf_member_name, member_name, 254);
                        char *arr_c = strstr(buf_member_name, "[");
                        if (arr_c == NULL) {
                            ERR("arr_c is NULL");
                        }
                        arr_c[0] = '\0';
                        char* arr_index_c = strstr(member_name, "[");
                        if (arr_index_c == NULL) {
                            ERR("arr_index_c is NULL");
                        }
                        node_arr_index = cut_array_index(arr_index_c);
                        member_name = buf_member_name;
                    }
                    if (json_object_has_member(member_obj, member_name)) {
                        member_node = json_object_get_member(member_obj, member_name);
                    } else {
                        ERR("Invalid member: %s", member_name);
                        keep_d = FALSE;
                    }
                }
                break;
            case JSON_NODE_ARRAY:
                //DBG("JSON_NODE_ARRAY");
                {
                  JsonArray* member_arr = json_node_get_array(member_node);
                  int length = json_array_get_length(member_arr);
                  if (node_arr_index < length) {
                      member_node = json_array_get_element(member_arr, node_arr_index);
                  } else {
                      ERR("Out of index(index, max): (%d, %d)", node_arr_index, length);
                  }
                }
                break;
            case JSON_NODE_VALUE:
                //DBG("JSON_NODE_VALUE");
                {
                    keep_d = FALSE;
                    return_v = __read_node_value(node_vt, member_node);
                }
                break;
            case JSON_NODE_NULL:
                //DBG("JSON_NODE_NULL");
                break;
            default:
                WRN("Unsupported type: %d", member_type);
                break;
        }
    }

    free(buf_node_path);

    return return_v;
}

static gboolean __write_node_value(node_value_type_e write_vt, JsonNode* node, void* value)
{
    GType value_type = json_node_get_value_type(node);
    switch (value_type) {
        case G_TYPE_STRING:
            {
                const char* v = (const char*)value;
                json_node_set_string(node, v);
            }
            break;
        default:
            // debug
            WRN("Unsupported type: %lu", value_type);
            __gtype_print(value_type);
            break;
    }

    return TRUE;
}

static gboolean __object_set_member_value(node_value_type_e node_vt, JsonObject* obj, const char* node_path, void* user_value)
{
    // parse node name with token
    char* buf_node_path = strdup(node_path);
    char* member_name = NULL;
    JsonObject* member_obj = obj;

    member_name = strtok(buf_node_path, ".");
    if (json_object_has_member(member_obj, member_name) == FALSE) {
        ERR("Invalid member: %s", member_name);
        free(buf_node_path);
        return FALSE;
    }

    JsonNode* member_node = json_object_get_member(obj, member_name);
    gboolean keep_d = TRUE;
    char buf_member_name[256] = {0, };
    int node_arr_index = 0;
    for (; member_name && keep_d;) {
        DBG("member_name: %s", member_name);
        JsonNodeType member_type = json_node_get_node_type(member_node);
        switch (member_type) {
            case JSON_NODE_OBJECT:
                DBG("JSON_NODE_OBJECT");
                {
                    member_obj = json_node_get_object(member_node);
                    member_name = strtok(NULL, ".");
                    if (__chk_has_array_format(member_name)) {
                        WRN("member_name has array format");
                        strncpy(buf_member_name, member_name, 254);
                        char* arr_c = strstr(buf_member_name, "[");
                        if (arr_c == NULL) {
                            ERR("arr_c is NULL");
                        }
                        arr_c[0] = '\0';
                        char* arr_index_c = strstr(member_name, "[");
                        if (arr_index_c == NULL) {
                            ERR("arr_index_c is NULL");
                        }
                        node_arr_index = cut_array_index(arr_index_c);
                        member_name = buf_member_name;
                    }
                    if (json_object_has_member(member_obj, member_name)) {
                        member_node = json_object_get_member(member_obj, member_name);
                    } else {
                        ERR("Invalid member: %s", member_name);
                        keep_d = FALSE;
                    }
                }
                break;
            case JSON_NODE_ARRAY:
                DBG("JSON_NODE_ARRAY");
                {
                    JsonArray* member_arr =  json_node_get_array(member_node);
                    int length = json_array_get_length(member_arr);
                    if (node_arr_index < length) {
                        member_node = json_array_get_element(member_arr, node_arr_index);
                    } else {
                        ERR("Out of index(index, max): (%d, %d)", node_arr_index, length);
                    }
                }
                break;
            case JSON_NODE_VALUE:
                DBG("JSON_NODE_VALUE");
                {
                    keep_d = FALSE;
                    __write_node_value(node_vt, member_node, user_value);
                }
                break;
            case JSON_NODE_NULL:
                DBG("JSON_NODE_NULL");
                break;
            default:
                WRN("Unsupported type: %d", member_type);
                break;
        }
    }

    free(buf_node_path);
        
    return TRUE;
}

const gchar* jm_object_get_string(JsonObject* obj, const char* node_path)
{
    void* n_value = __object_get_member_value(NODE_VALUE_TYPE_STRING, obj, node_path);
    return (const gchar*)n_value;
}

int jm_object_get_int(JsonObject* obj, const char* node_path)
{
    void* n_value = __object_get_member_value(NODE_VALUE_TYPE_INT, obj, node_path);
    return *((int*)n_value);
}

double jm_object_get_double(JsonObject* obj, const char* node_path)
{
    void* n_value = __object_get_member_value(NODE_VALUE_TYPE_DOUBLE, obj, node_path);
    return *((double*)n_value);
}

gboolean jm_object_get_boolean(JsonObject* obj, const char* node_path)
{
    void* n_value = __object_get_member_value(NODE_VALUE_TYPE_BOOLEAN, obj, node_path);
    return *((gboolean*)n_value);
}

gboolean jm_object_set_string(JsonObject* obj, const char* node_path, const char* value)
{
    __object_set_member_value(NODE_VALUE_TYPE_STRING, obj, node_path, (void*)value);
    return TRUE;
}
