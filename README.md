# json-mod

This project will help you when you access json node

your code will be shortly to access like below

jm_object_get_string_member(json_object, 3, "widget", "debug", "image");

this code will return value of string memeber for below path
"widget.debug.image"

How to compile source
$ gcc -W -Werror -Wall jm_test.c -o jm_test `pkg-config --cflags --libs glib-2.0 json-glib-1.0`

How to run sample code
$ ./jm_test sample1.json


