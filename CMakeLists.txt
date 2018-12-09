project("jm_test")
include(FindPkgConfig)
pkg_search_module(GLIB2 REQUIRED glib-2.0)

include_directories(${GLIB2_INCLUDE_DIRS})

pkg_search_module(JSON-GLIB REQUIRED json-glib-1.0)
include_directories(${JSON-GLIB_INCLUDE_DIRS})
ADD_EXECUTABLE ( ${PROJECT_NAME} jm_test.c json-mod.c )
target_link_libraries(${PROJECT_NAME} ${GLIB2_LIBRARIES} ${JSON-GLIB_LIBRARIES})
