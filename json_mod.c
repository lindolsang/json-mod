#include <stdlib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <stdio.h>
#include <string.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "juu"

#define DBG(fmt, arg...) \
    ({ do { \
        printf("D - " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
    } while (0); })
#define ERR(fmt, arg...) \
    ({ do { \
        printf("E - " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
    } while (0); })
#define INF(fmt, arg...) \
    ({ do { \
        printf("I - " LOG_TAG " %s: %s(%d) > " fmt "\n", __FILE__, __func__, __LINE__, ##arg); \
    } while (0); }) 

JsonNode* json_search_id(JsonObject *obj_root, const char* unique_id);
int json_write_to_file(JsonNode *root_node, const char* path);

gboolean json_write_to_file(JsonNode *root_node, const char *path)
{
    JsonGenerator *gen = json_generator_new();
    json_generator_set_root(gen, root_node);

    GError *error = NULL;
    gboolean result = FALSE;

    result  = json_generator_to_file(gen, path, &error);
    if (error) {
        ERR("Error - can not make json file: %s", error->message);
        g_error_free(error);
    }

    g_object_unref(gen);

    return result;
}

void json_replace_value_with_string(JsonNode *parent, const char *member, const char *new)
{

    JsonObject *obj_parent = json_node_get_object(parent);
    if (json_object_has_member(obj_parent, member)) {
        json_object_set_string_member(obj_parent, member, new);
    }
}

void test_my_json_operation(JsonObject *object)
{
    gboolean has_member = json_object_has_member(object, "glossary");
    if (has_member) {

        JsonObject *obj_glossary = json_object_get_object_member(object, "glossary");

        JsonNode* member = json_search_id(obj_glossary, "idi");
        //json_node_set_string(member, "L SGML"); 
        JsonNode *parent = json_node_get_parent(member);
        const char *target_node = json_node_get_string(member);
        const char *new_value = "juu test";
        json_replace_value_with_string(parent, target_node, new_value);
        
        JsonNode *tmp_node = json_node_new(JSON_NODE_OBJECT);
        json_node_set_object(tmp_node, object);
        json_write_to_file(tmp_node, "test_new.json");
        json_node_free(tmp_node);
    }
}

JsonNode* json_search_id(JsonObject *obj_root, const char* unique_id)
{
    GList* list_members = json_object_get_members(obj_root);
    GList* first_header = list_members;
    JsonNode *found = NULL;

    for (; list_members != NULL; list_members = list_members->next) {
        const char *member_name = (const char*)list_members->data;
        DBG("member: %s", member_name);
        if (strcmp(member_name, unique_id) == 0) {
            INF("found: [%s] ", unique_id);
            found = json_object_get_member(obj_root, member_name);
            break;
        } else {

            JsonNode *node_member = json_object_get_member(obj_root, member_name);
            JsonNodeType member_type = json_node_get_node_type(node_member);

            if (member_type == JSON_NODE_OBJECT) {
                JsonObject *obj_member = json_node_get_object(node_member);
                found = json_search_id(obj_member, unique_id);
            } else if (member_type == JSON_NODE_ARRAY) {
                DBG("need to implement for array");
            }
        }
    }

    g_list_free(first_header);
    return found;
}

int
main (int argc, char *argv[])
{
  JsonParser *parser;
  JsonNode *root;
  GError *error;

  if (argc < 2)
    {
      g_print ("Usage: test <filename.json>\n");
      return EXIT_FAILURE;
    }

  parser = json_parser_new ();

  error = NULL;
  json_parser_load_from_file (parser, argv[1], &error);
  if (error)
    {
      g_print ("Unable to parse `%s': %s\n", argv[1], error->message);
      g_error_free (error);
      g_object_unref (parser);
      return EXIT_FAILURE;
    }

  root = json_parser_get_root (parser);

  /* manipulate the object tree and then exit */
  JsonObject *object = json_node_get_object(root);
  test_my_json_operation(object);

  g_object_unref (parser);

  return EXIT_SUCCESS;
}
