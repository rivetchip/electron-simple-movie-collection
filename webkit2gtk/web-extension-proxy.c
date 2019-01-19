#include <config.h> //build generated
#include <stdlib.h>
#include <glib.h>
#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>

static char *storageFilename;
static char *storageFolder;
static char *storagePosters;

//todo: use glib socket to commucate with main for ask open file via gtk chooser
//todo: freed * on jsinterface

static JSCValue* javascriptinterface_open_collection(WebKitWebExtension *extension) {
    JSCContext *context = jsc_context_get_current();

    GError *error = NULL;
    char *contents = NULL;

    bool success = (
        // create save path if not set
        g_mkdir_with_parents(storageFolder, 0755) == 0 &&
        g_file_get_contents(
            g_build_filename(storageFolder, storageFilename, NULL),
            &contents, NULL, &error
        )
    );

    if(success && contents) {
        return jsc_value_new_string(context, contents);
    }

    if(error != NULL) {
        g_warning("extension:cannot open collection : %s", error->message);
        g_clear_error(&error);
    }

    return jsc_value_new_boolean(context, FALSE);
}

static JSCValue* javascriptinterface_save_collection(WebKitWebExtension *extension, const char *contents) {
    JSCContext *context = jsc_context_get_current();

    GError *error = NULL;

    bool success = (
        // create save path if not set
        g_mkdir_with_parents(storageFolder, 0755) == 0 &&
        g_file_set_contents(
            g_build_filename(storageFolder, storageFilename, NULL),
            contents, -1, &error
        )
    );

    if(success) {
        return jsc_value_new_boolean(context, TRUE);
    }

    if(error != NULL) {
        g_warning("extension:cannot save collection : %s", error->message);
        g_clear_error(&error);
    }

    return jsc_value_new_boolean(context, FALSE);
}

static JSCValue* javascriptinterface_get_poster(WebKitWebExtension *extension, const char *poster_name) {
    JSCContext *context = jsc_context_get_current();

    char *filename = g_build_filename(storagePosters, poster_name, NULL);

    bool success = (g_file_test(filename, G_FILE_TEST_IS_REGULAR));

    if(success) {
        // return full poster path
        return jsc_value_new_string(context, g_build_filename("file://", filename, NULL));
    }

    return jsc_value_new_boolean(context, FALSE);
}

static JSCValue* javascriptinterface_save_poster(WebKitWebExtension *extension, const char *poster_name, const char *contents) {
    JSCContext *context = jsc_context_get_current();

    GError *error = NULL;

    char *filename = g_build_filename(storagePosters, poster_name, NULL);

    bool success = (
        // create save path if not set
        g_mkdir_with_parents(storagePosters, 0755) == 0 &&
        g_file_set_contents(
            g_build_filename(storagePosters, filename, NULL),
            contents, -1, &error
        )
    );

    if(success) {
        // return full poster path
        return jsc_value_new_string(context, g_build_filename("file://", filename, NULL));
    }

    if(error != NULL) {
        g_warning("extension:cannot save poster : %s", error->message);
        g_clear_error(&error);
    }

    return jsc_value_new_boolean(context, FALSE);
}

static bool javascriptinterface_get_developer_mode(WebKitWebExtension *extension) {
    return PACKAGE_DEVELOPER_MODE;
}




static void webkit_world_window_cleared_callback(
    WebKitScriptWorld *world,
    WebKitWebPage *page,
    WebKitFrame *frame,
    WebKitWebExtension *extension // user_data
) {
    g_assert(WEBKIT_IS_WEB_EXTENSION(extension));

    // extend javascriptCore APi

    JSCContext *jsc_context = webkit_frame_get_js_context_for_script_world(frame, world);
    JSCValue *jsc_globalObject = jsc_context_get_global_object(jsc_context);

    JSCClass *js_class = jsc_context_register_class(jsc_context, "WebkitgtkInterface", NULL, NULL, NULL);

    jsc_class_add_property(
        js_class, "developer_mode", G_TYPE_BOOLEAN,
        G_CALLBACK(javascriptinterface_get_developer_mode),
        NULL, // callback
        NULL, // user_data
        NULL // destroy_notify
    );

    jsc_class_add_method(
        js_class, "openCollection",
        G_CALLBACK(javascriptinterface_open_collection), NULL,
        NULL, // destroy_notify
        JSC_TYPE_VALUE, //G_TYPE_STRING // return type
        0, G_TYPE_NONE // args number and types
    );

    jsc_class_add_method(
        js_class, "saveCollection",
        G_CALLBACK(javascriptinterface_save_collection), NULL,
        NULL, // destroy_notify
        JSC_TYPE_VALUE, // return type
        1, G_TYPE_STRING // args number and types
    );

    jsc_class_add_method(
        js_class, "getPoster",
        G_CALLBACK(javascriptinterface_get_poster), NULL,
        NULL, // destroy_notify
        JSC_TYPE_VALUE, // return type
        1, G_TYPE_STRING // args number and types
    );

    jsc_class_add_method(
        js_class, "savePoster",
        G_CALLBACK(javascriptinterface_save_poster), NULL,
        NULL, // destroy_notify
        JSC_TYPE_VALUE, // return type
        2, G_TYPE_STRING, G_TYPE_STRING // args number and types
    );

    JSCValue *js_interface = jsc_value_new_object(jsc_context, extension, js_class);

    jsc_value_object_set_property(jsc_globalObject, "WebkitgtkInterface", js_interface);
    
    g_object_unref(js_interface);
}


static void web_page_document_loaded_callback(WebKitWebPage *web_page) {

    g_message("extension:Page %" G_GUINT64_FORMAT " created for %s",
        webkit_web_page_get_id(web_page),
        webkit_web_page_get_uri(web_page)
    );
}

static gboolean web_page_send_request_callback(
    WebKitWebPage *web_page,
    WebKitURIRequest *request,
    WebKitURIResponse *redirected_response
) {
    const char* request_method = webkit_uri_request_get_http_method(request);
    const char* request_uri = webkit_uri_request_get_uri(request);

    g_message("extension:request %.0s %s", request_method, request_uri);

    return FALSE; // continue request
}

static void web_page_console_message_callback(WebKitWebPage *web_page, WebKitConsoleMessage *console_message) {
    // fixme: console-log not send to stdout when ephemeral context ??

    g_message("extension:console #%i - %s",
        webkit_console_message_get_line(console_message),
        webkit_console_message_get_text(console_message)
    );
}


static void web_page_created_callback(WebKitWebExtension *extension, WebKitWebPage *web_page) {

    g_signal_connect(web_page, "console-message-sent", G_CALLBACK(web_page_console_message_callback), NULL);
    g_signal_connect(web_page, "document-loaded", G_CALLBACK(web_page_document_loaded_callback), NULL);
    g_signal_connect(web_page, "send-request", G_CALLBACK(web_page_send_request_callback), NULL);
}



// main
G_MODULE_EXPORT void webkit_web_extension_initialize_with_user_data(WebKitWebExtension *extension, GVariant *user_data) {
    
    g_assert(WEBKIT_IS_WEB_EXTENSION(extension));

    const int unique_id;
    const char *appid;
    const char *webext_file;

    g_variant_get(user_data, "(iss)", &unique_id, &appid, &webext_file);

    // set user defined storage todo
    storageFilename = "moviecollection.json";
    storageFolder = g_build_filename(g_get_user_data_dir(), appid, NULL);
    storagePosters = g_build_filename(storageFolder, "posters", NULL);

    #if PACKAGE_DEVELOPER_MODE
        storageFolder = g_build_filename(PACKAGE_BUILD_ROOT, "test-collection", NULL);
        storagePosters = g_build_filename(PACKAGE_BUILD_ROOT, "test-posters", NULL);
    #endif

    WebKitScriptWorld *webkit_world = webkit_script_world_get_default(); //webkit_script_world_new_with_name

    g_signal_connect(extension, "page-created", G_CALLBACK(web_page_created_callback), NULL);
    g_signal_connect(webkit_world, "window-object-cleared", G_CALLBACK(webkit_world_window_cleared_callback), extension);
}

