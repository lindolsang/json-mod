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
        printf("E " LOG_TAG " %s: %s(%d) > " fmt, __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define DBG(fmt, arg...) \
    ({ do { \
        printf("D " LOG_TAG " %s: %s(%d) > " fmt, __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define INF(fmt, arg...) \
    ({ do { \
        printf("I " LOG_TAG " %s: %s(%d) > " fmt, __FILE__, __func__, __LINE__, ##arg); \
     } while (0); })

#define WRN(fmt, arg...) \
    ({ do { \
        printf("W " LOG_TAG " %s: %s(%d) > " fmt, __FILE__, __func__, __LINE__, ##arg); \
    } while (0); })

const char* jm_object_get_string_member(JsonObject* obj, int length, ...);
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
        ERR("Invalid member: %s\n", member_str);
        return NULL;
    }

    JsonNode* member_node = json_object_get_member(c_obj, member_str);

    for (; i < length && return_value == NULL; i++) {
        JsonNodeType node_type = json_node_get_node_type(member_node);
        switch (node_type) {
            case JSON_NODE_OBJECT:
                DBG("JSON_NODE_OBJECT\n");
                {
                    c_obj = json_node_get_object(member_node);
                    member_str = va_arg(valist, char*);
                    if (json_object_has_member(c_obj, member_str)) {
                        member_node = json_object_get_member(c_obj, member_str);
                    } else {
                        ERR("else!\n");
                    }
                }
                break;
            case JSON_NODE_ARRAY:
                DBG("JSON_NODE_ARRAY\n");
                {
                    JsonArray* c_arr = json_node_get_array(member_node);
                    member_str = va_arg(valist, char*);
                    int arr_index = cut_array_index(member_str);
                    member_node = json_array_get_element(c_arr, arr_index);
                }
                break;
            case JSON_NODE_VALUE:
                DBG("JSON_NODE_VALUE\n");
                {
                    // https://developer.gnome.org/gobject/stable/gobject-Type-Information.html#GType
                    GType value_type = json_node_get_value_type(member_node);
                    if (value_type == G_TYPE_STRING) {
                        return_value = json_node_get_string(member_node);
                    }
                }
                break;
            case JSON_NODE_NULL:
                DBG("JSON_NODE_NULL\n");
                break;
            default:
                WRN("Unsupported type\n");
        }
    }

    va_end(valist);

    return (i == length) ? return_value : NULL;
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
    DBG("title: %s\n", title);
    const char* invalid_test = jm_object_get_string_member(obj, 3, "widget", "debug", "image");
    DBG("message: %s\n", invalid_test);
    const char* hello_0 = jm_object_get_string_member(obj, 3, "widget", "welcome", "[1]");
    DBG("welcome: %s\n", hello_0);
    const char* category = jm_object_get_string_member(obj, 6, "widget", "notification", "[1]", "type", "category", "[0]");
    DBG("category: %s\n", category);
    const char* id = jm_object_get_string_member(obj, 5, "widget", "notification", "[1]", "type", "id");
    DBG("id: %s\n", id);

    g_object_unref(parser);
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{

    if (argc < 2) {
        ERR("Usage: test <filename.json>\n");
        return EXIT_FAILURE;
    }

    test_get(argv[1]);

    return EXIT_SUCCESS;
}
