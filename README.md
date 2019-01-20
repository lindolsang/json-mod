# json-mod

This project will help you when you access json node

your code will be shortly to access like below

jm_object_get_string(json_object, "widget.window.title");

this code will return value of string memeber for below path
"widget.window.title"

sample json
```json
{
    "widget": {
        "debug": "on",
        "window": {
            "title": "Sample Widget",
            "name": "main_window",
            "width": 500,
            "height": 500
        }
    }
}
```

and when you calls function like below
```c
jm_object_get_string(json_object, "widget.window.title");
```

you can get below value
```
Sample Widget
```

How to compile source
```shell
$ gcc -W -Werror -Wall jm_test.c -o jm_test `pkg-config --cflags --libs glib-2.0 json-glib-1.0`
```
or
```shell
$ cmake CMakeLists.txt
$ make
```

How to run sample code
```shell
$ ./jm_test sample1.json
```
