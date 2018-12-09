#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

#undef LOG_TAG
#define LOG_TAG "jm"

#define ERR(fmt, arg...) \
    ({ do { \
        printf("E " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define DBG(fmt, arg...) \
    ({ do { \
        printf("D " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define INF(fmt, arg...) \
    ({ do { \
        printf("I " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define WRN(fmt, arg...) \
    ({ do { \
        printf("W " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
    } while (0); })

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

int test_get(const char* file_name)
{
    JsonParser *parser;
    JsonNode *root;
    GError *error;

    parser = json_parser_new();
    error = NULL;
    json_parser_load_from_file(parser, file_name, &error);
    if (error) {
        g_print("Unable to parse `%s`: %s\n", file_name, error->message);
        g_error_free(error);
        g_object_unref(parser);
        return EXIT_FAILURE;
    }

    root = json_parser_get_root(parser);
    /* manipulate the object tree and then exit */
    JsonObject* obj = json_node_get_object(root);
    const char* title = jm_object_get_string_member(obj, 3, "widget", "window", "title");
    DBG("title: %s", title);
    const char* invalid_test = jm_object_get_string_member(obj, 3, "widget", "debug", "image");
    DBG("message: %s", invalid_test);
    const char* hello_0 = jm_object_get_string_member(obj, 3, "widget", "welcome", "[1]");
    DBG("welcome: %s", hello_0);
    const char* category = jm_object_get_string_member(obj, 6, "widget", "notification", "[1]", "type", "category", "[0]");
    DBG("category: %s", category);
    const char* id = jm_object_get_string_member(obj, 5, "widget", "notification", "[1]", "type", "id");
    DBG("id: %s", id);

    /* invalid case */
    const char* name = jm_object_get_string_member(obj, 3, "widget", "debug", "title");
    DBG("name: %s", name);

    g_object_unref(parser);
    return EXIT_SUCCESS;
}

int json_save_file(JsonNode* root, const char* file_name)
{
    JsonGenerator *gen = json_generator_new();
    json_generator_set_root(gen, root);
    GError *error = NULL;
    json_generator_to_file(gen, file_name, &error);
    if (error) {
        g_print("Unable to save `%s`: %s\n", file_name, error->message);
        g_error_free(error);
        g_object_unref(gen);
        return EXIT_FAILURE;
    }
    g_object_unref(gen);
    return EXIT_SUCCESS;
}

int test_set(const char* file_name)
{
    JsonParser *parser = NULL;
    JsonNode *root = NULL;
    GError *error = NULL;

    parser = json_parser_new();
    error = NULL;
    json_parser_load_from_file(parser, file_name, &error);
    if (error) {
        g_print("Unable to parse `%s`: %s\n", file_name, error->message);
        g_error_free(error);
        g_object_unref(parser);
        return EXIT_FAILURE;
    }

    root = json_parser_get_root(parser);
    /* manipulate the object tree and then exit */
    JsonObject* obj = json_node_get_object(root);
    gboolean is_set = jm_object_set_string_member(obj, "Test Title", 3, "widget", "window", "title");
    DBG("seted: %s", (is_set ? "true" : "false"));

    jm_object_set_string_member(obj, "Good morning!", 3, "widget", "welcome", "[1]");
    jm_object_set_string_member(obj, "Wallpaper", 6, "widget", "notification", "[0]", "type", "category", "[0]");

    /* invalid case */
    is_set = jm_object_set_string_member(obj, "Keyboard", 6, "widget", "debug", "[0]", "type", "category", "[1]");
    DBG("seted: %s", (is_set ? "true" : "false"));
    is_set = jm_object_set_string_member(obj, "button2", 4, "widget", "handler", "click", "parametersS");
    DBG("seted: %s", (is_set ? "true" : "false"));
    is_set = jm_object_set_string_member(obj, "button2", 4, "widget", "handler", "click", "parameters");

    /* save output */
    json_save_file(root, "sample_out.json");

    g_object_unref(parser);
    return EXIT_SUCCESS;
}

int test_int_get(const char* file_name)
{
    JsonParser *parser;
    JsonNode *root;
    GError *error;

    parser = json_parser_new();

    error = NULL;
    json_parser_load_from_file(parser, file_name, &error);
    if (error) {
        g_print("Unable to parse `%s': %s\n", file_name, error->message);
        g_error_free(error);
        g_object_unref(parser);
        return EXIT_FAILURE;
    }

    root = json_parser_get_root(parser);

    /* manipulate the object tree and then exit */
    JsonObject* obj = json_node_get_object(root);

    int width = jm_object_get_int_member(obj, 3, "widget", "window", "width");
    int height = jm_object_get_int_member(obj, 3, "widget", "window", "height");
    DBG("width: %d, height: %d", width, height);
    int h_offset = jm_object_get_int_member(obj, 3, "widget", "image", "hOffset");
    int v_offset = jm_object_get_int_member(obj, 3, "widget", "image", "vOffset");
    DBG("hOffset: %d, vOffset: %d", h_offset, v_offset);

    g_object_unref(parser);
    return EXIT_SUCCESS;
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

/**
 This api will make the new JsonNode like clone
 Even if @node has immutable property, returned value will not have immutable.

 @param node the pointer that is used for source
 @return the pointer of JsonNode that has cloned
 @remark You should release returned value using json_node_unref()
  */
JsonNode* jm_node_clone(JsonNode* node)
{
    JsonBuilder* builder = json_builder_new();
    __jm_json_node_build(node, builder);
    JsonNode* return_node = json_builder_get_root(builder);
    g_object_unref(builder);

    return return_node;
}

int test_clone(const char* file_name)
{
    JsonParser *parser;
    JsonNode *root;
    GError *error;

    parser = json_parser_new();
    error = NULL;
    json_parser_load_from_file(parser, file_name, &error);
    if (error) {
        ERR("Unable to parse `%s': %s", file_name, error->message);
        g_error_free(error);
        g_object_unref(parser);
        return EXIT_FAILURE;
    }

    root = json_parser_get_root(parser);

    /* manipulate the object tree and then exit */
    JsonNode* root_clone = jm_node_clone(root); 
    
    g_object_unref(parser);

    // test output
    json_save_file(root_clone, "sample_clone.json");

    json_node_unref(root_clone);

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{

    if (argc < 2) {
        ERR("Usage: test <filename.json>");
        return EXIT_FAILURE;
    }

    //test_get(argv[1]);
    //test_set(argv[1]);
    //test_int_get(argv[1]);
    test_clone(argv[1]);

    return EXIT_SUCCESS;
}
