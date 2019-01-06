
#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>


static JSCValue* javascriptinterface_open_collection(WebKitWebExtension *extension) {

    JSCContext *context = jsc_context_get_current();

    GError *error_read = NULL;

    char *content = NULL;
    g_file_get_contents("/home/fox/Bureau/vscode", &content, NULL, &error_read);

    if(error_read != NULL) {
        g_warning("extension:cannot open collection : %s", error_read->message);
        g_clear_error(&error_read);
    }

    return jsc_value_new_string(context, content);
}

static JSCValue* javascriptinterface_save_collection(WebKitWebExtension *extension, const char *collect_content) {

    JSCContext *context = jsc_context_get_current();

    bool success = FALSE;



    return jsc_value_new_boolean(context, success);
}

static JSCValue* javascriptinterface_get_poster(WebKitWebExtension *extension, const char *poster_name) {

    JSCContext *context = jsc_context_get_current();

    char *filename = NULL;



filename = "sqdqsdqq";


    return jsc_value_new_string(context, filename);
}

static JSCValue* javascriptinterface_save_poster(WebKitWebExtension *extension, const char *poster_name, const char *poster_content) {

    JSCContext *context = jsc_context_get_current();

    bool success = FALSE;



    return jsc_value_new_boolean(context, success);
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

    // jsc_class_add_property(js_class, "version", G_TYPE_INT, )

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
    const char *webextension_dir;

    g_variant_get(user_data, "(is)", &unique_id, &webextension_dir);


    WebKitScriptWorld *webkit_world = webkit_script_world_get_default(); //webkit_script_world_new_with_name

    g_signal_connect(extension, "page-created", G_CALLBACK(web_page_created_callback), NULL);
    g_signal_connect(webkit_world, "window-object-cleared", G_CALLBACK(webkit_world_window_cleared_callback), extension);
}

