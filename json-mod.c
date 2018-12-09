#include "json-mod.h"
#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

const char* jm_object_get_string_member(JsonObject* obj, int length, ...);
gboolean jm_object_set_string_member(JsonObject* obj, const char*, int length, ...);
int jm_object_get_int_member(JsonObject* obj, int length, ...);

int cut_array_index(const char* index_str) {
    char buf[10] = {0, };
    snprintf(buf, 10, "%s", index_str + 1);
    int length = strlen(buf);
    buf[length - 1] = '\0';
    return atoi(buf);
}

const char* jm_object_get_string_member(JsonObject* obj, int length, ...)
{
    va_list valist;
    int i = 0;
    const char* return_value = NULL;

    va_start(valist, length);

    JsonObject* c_obj = obj;
    char *member_str = va_arg(valist, char*);

    if (json_object_has_member(c_obj, member_str) == FALSE) {
        ERR("Invalid member: %s", member_str);
        return NULL;
    }

    JsonNode* member_node = json_object_get_member(c_obj, member_str);

    for (; i < length && return_value == NULL; i++) {
        JsonNodeType node_type = json_node_get_node_type(member_node);
        switch (node_type) {
            case JSON_NODE_OBJECT:
                DBG("JSON_NODE_OBJECT");
                {
                    c_obj = json_node_get_object(member_node);
                    member_str = va_arg(valist, char*);
                    if (json_object_has_member(c_obj, member_str)) {
                        member_node = json_object_get_member(c_obj, member_str);
                    } else {
                        ERR("Invalid member: %s", member_str);
                    }
                }
                break;
            case JSON_NODE_ARRAY:
                DBG("JSON_NODE_ARRAY");
                {
                    JsonArray* c_arr = json_node_get_array(member_node);
                    member_str = va_arg(valist, char*);
                    int arr_index = cut_array_index(member_str);
                    member_node = json_array_get_element(c_arr, arr_index);
                }
                break;
            case JSON_NODE_VALUE:
                DBG("JSON_NODE_VALUE");
                {
                    // https://developer.gnome.org/gobject/stable/gobject-Type-Information.html#GType
                    GType value_type = json_node_get_value_type(member_node);
                    if (value_type == G_TYPE_STRING) {
                        return_value = json_node_get_string(member_node);
                    }
                }
                break;
            case JSON_NODE_NULL:
                DBG("JSON_NODE_NULL");
                break;
            default:
                WRN("Unsupported type");
        }
    }

    va_end(valist);

    return (i == length) ? return_value : NULL;
}

gboolean jm_object_set_string_member(JsonObject* obj, const char* value, int length, ...)
{
    va_list valist;
    int i = 0;
    gboolean is_setted = FALSE;

    va_start(valist, length);

    JsonObject* c_obj = obj;
    char* member_str = va_arg(valist, char*);

    if (json_object_has_member(c_obj, member_str) == FALSE) {
        ERR("Invalid member: %s", member_str);
        return is_setted;
    }

    JsonNode* member_node = json_object_get_member(c_obj, member_str);
    gboolean path_is_valid = TRUE;

    for (; i < length && path_is_valid; i++) {
        JsonNodeType node_type = json_node_get_node_type(member_node);
        switch (node_type) {
            case JSON_NODE_OBJECT:
                DBG("JSON_NODE_OBJECT");
                {
                    if (i + 1 == length) {
                        DBG("end of path");
                    } else {
                        c_obj = json_node_get_object(member_node);
                        member_str = va_arg(valist, char*);
                        if (json_object_has_member(c_obj, member_str)) {
                            member_node = json_object_get_member(c_obj, member_str);
                        } else {
                            ERR("Invalid member: %s", member_str);
                            path_is_valid = FALSE;
                        }
                    }
                }
                break;
            case JSON_NODE_ARRAY:
                DBG("JSON_NODE_ARRAY");
                {
                    JsonArray* c_arr = json_node_get_array(member_node);
                    member_str = va_arg(valist, char*);
                    int arr_index = cut_array_index(member_str);
                    member_node = json_array_get_element(c_arr, arr_index);
                }
                break;
            case JSON_NODE_VALUE:
                DBG("JSON_NODE_VALUE");
                {
                    if (i + 1 < length) {
                        ERR("Invalid path: %s", member_str);
                        path_is_valid = FALSE;
                    } else {
                        json_node_set_string(member_node, value);
                        is_setted = TRUE;
                    }
                }
                break;
            case JSON_NODE_NULL:
                DBG("JSON_NODE_NULL");
                break;
            default:
                WRN("Unsupported type");
        }
    }

    va_end(valist);

    return is_setted;
}

int jm_object_get_int_member(JsonObject* obj, int length, ...)
{
    va_list valist;
    int i = 0;
    int return_value = 0;

    va_start(valist, length);

    char *member_name = va_arg(valist, char*);
    if (json_object_has_member(obj, member_name) == FALSE) {
        ERR("Invalid member: %s", member_name);
        return 0;
    }

    JsonNode* member_node = json_object_get_member(obj, member_name);

    for (; i < length; i++) {
        DBG("member: %s", member_name);

        JsonNodeType node_type = json_node_get_node_type(member_node);
        switch (node_type) {
            case JSON_NODE_OBJECT:
                DBG("JSON_NODE_OBJECT");
                {
                    member_name = va_arg(valist, char *);
                    JsonObject* member_obj = json_node_get_object(member_node);
                    if (json_object_has_member(member_obj, member_name) == FALSE) {
                        ERR("Invalid member: %s", member_name);
                        return 0;
                    }
                    member_node = json_object_get_member(member_obj, member_name);
                }
                break;
            case JSON_NODE_ARRAY:
                DBG("JSNO_NODE_ARRAY");
                break;
            case JSON_NODE_VALUE:
                DBG("JSON_NODE_VALUE");
                {
                    return_value = json_node_get_int(member_node);
                }
                break;
            case JSON_NODE_NULL:
                DBG("JSON_NODE_NULL");
                break;
            default:
                WRN("Unsupported type");
        }
    }

    va_end(valist);
    return return_value;
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
