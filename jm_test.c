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

int test_get(const char* file_name)
{
    JsonParser* parser;
    JsonNode* root;
    GError* error;

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
    JsonObject* root_obj = json_node_get_object(root);
    DBG("widget.debug = %s", jm_object_get_string(root_obj, "widget.debug"));
    DBG("widget.splash_window.splash.size = %d", jm_object_get_int(root_obj, "widget.splash_window.splash.size"));
    DBG("widget.image.hOffset = %d", jm_object_get_int(root_obj, "widget.image.hOffset"));
    DBG("widget.splash_window.property.duration = %f", jm_object_get_double(root_obj, "widget.splash_window.property.duration"));
    DBG("widget.splash_window.property.animation.blink = %s", jm_object_get_boolean(root_obj, "widget.splash_window.property.animation.blink") ? "true" : "false");
    DBG("widget.notification[0].type.id = %s", jm_object_get_string(root_obj, "widget.notification[0].type.id"));
    DBG("widget.notification[0].type.category[1] = %s", jm_object_get_string(root_obj, "widget.notification[0].type.category[1]"));
    DBG("widget.notification[1].type.sub_category[0].value = %s", jm_object_get_boolean(root_obj, "widget.notification[1].type.sub_category[0].value") ? "true" : "false");
    DBG("widget.notification[1].type.sub_category[1].value = %f", jm_object_get_double(root_obj, "widget.notification[1].type.sub_category[1].value"));
    DBG("widget.notification[1].type.sub_category[2].value = %s", jm_object_get_string(root_obj, "widget.notification[1].type.sub_category[2].value"));
    DBG("widget.notification[1].type.sub_category[3].value = %d", jm_object_get_int(root_obj, "widget.notification[1].type.sub_category[3].value"));

    g_object_unref(parser);
    return EXIT_SUCCESS;
}

int test_set(const char* file)
{
    JsonParser* parser = NULL;
    JsonNode* root = NULL;
    GError *error = NULL;

    parser = json_parser_new();
    json_parser_load_from_file(parser, file, &error);
    if (error) {
        ERR("Unable to parse `%s': %s\n", file, error->message);
        g_error_free(error);
        g_object_unref(parser);
        return EXIT_FAILURE;
    }

    root = json_parser_get_root(parser);

    /* manipulate the object tree and then exit */
    JsonNode* clone = jm_node_clone(root);
    g_object_unref(parser);

    /* test set */
    JsonObject* root_obj = json_node_get_object(clone);
    jm_object_set_string(root_obj, "widget.debug", "off");

    json_save_file(clone, "sample1_set.json");
    json_node_unref(clone);

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{

    if (argc < 2) {
        ERR("Usage: test <filename.json>");
        return EXIT_FAILURE;
    }

    test_get(argv[1]);
    test_set(argv[1]);
    test_clone(argv[1]);

    return EXIT_SUCCESS;
}
