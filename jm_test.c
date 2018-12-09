#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>

#include "json-mod.h"
#include "log.h"

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
