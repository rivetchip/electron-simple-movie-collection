/*
sudo dnf install gtk3-devel gstreamer-devel clutter-devel webkitgtk3-devel libgda-devel gobject-introspection-devel
webkit2gtk3-devel

gcc moviecollection.c -o main `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0` && ./main
*/


#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// #include <glib.h>

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <JavaScriptCore/JavaScript.h>


static void destroy_window_callback(
    GtkWidget* widget,
    GtkWidget* window
);
static gboolean close_webview_callback(
    WebKitWebView *webView,
    GtkWidget* window
);

JSClassRef WebAppInterface();


static void window_object_cleared_callback()

{

}



int main(int argc, char* argv[]) {

    bool is_debug = argv[1] != NULL && strcmp(argv[1], "--debug") == 0;

    // Initialize GTK+
    gtk_init(&argc, &argv);

    // Create an 800x600 window that will contain the browser instance
    GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    // Create a browser instance
    WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());


    // Set GTK window options
    gtk_window_set_title(GTK_WINDOW(main_window), "Movie Collection");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(main_window), true);

    // Put the browser area into the main window
    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(webview));

    // Set up callbacks so that if either the main window or the browser instance is
    // closed, the program will exit
    g_signal_connect(main_window, "destroy", G_CALLBACK(destroy_window_callback), NULL);
    g_signal_connect(webview, "close", G_CALLBACK(close_webview_callback), main_window);

    // Webview settings (debug)
    WebKitSettings *websettings = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(webview));
    webkit_settings_set_default_charset(websettings, "utf8");
    webkit_settings_set_enable_javascript(websettings, true);

    // if(is_debug) {
        webkit_settings_set_enable_write_console_messages_to_stdout(websettings, true);
        webkit_settings_set_enable_developer_extras(websettings, true);
    // }

    // connect the window object with custom classes to JavaScriptCore
    g_signal_connect(G_OBJECT(webview), "window-object-cleared", G_CALLBACK(window_object_cleared_callback), NULL);




    // JSObjectRef contaxt = jsc_value_get_context();

    // JSObjectRef globalObject = JSContextGetGlobalObject(webcontext);
	// JSObjectRef webAppInterfaceObject = JSObjectMake(webcontext, WebAppInterface(), NULL);

	// JSObjectSetProperty(
    //     webcontext, globalObject,
    //     JSStringCreateWithUTF8CString("WebkitgtkInterface"),
    //     webAppInterfaceObject,
    //     kJSPropertyAttributeNone,
    //     NULL
    // );








// guint64 webview_pageid = webkit_web_view_get_page_id(webview);

// JSCContext *jsccontext =  webkit_frame_get_js_context(webkit_frame);


/*

JSContextRef globalContext = JSGlobalContextCreate(NULL);

JSObjectRef globalObject = JSContextGetGlobalObject(globalContext);

JSObjectRef webAppInterfaceObject = JSObjectMake(globalContext, WebAppInterface(), NULL);

JSObjectSetProperty(
    globalContext, globalObject,
    JSStringCreateWithUTF8CString("WebkitgtkInterface"),
    webAppInterfaceObject,
    kJSPropertyAttributeNone,
    NULL
);


JSValueRef err = NULL;
JSStringRef str_script = JSStringCreateWithUTF8CString("return WebkitgtkInterface.openCollection();");

JSValueRef ret = JSEvaluateScript(globalContext, str_script, NULL, NULL, 0, &err);

bool response = JSValueToBoolean(globalContext, ret);

if(response) {
    g_message("ok");
} else {
    g_message("nok");
}


if(err != NULL) {
    g_message("error");
 JSStringRef pname = JSStringCreateWithUTF8CString("message");
 JSValueRef value = JSObjectGetProperty(globalContext, err, pname, NULL);
}


*/







    // Load a web page into the browser instance
    char cwd[256];
    getcwd(cwd, sizeof(cwd));

    char webview_page[256];
    sprintf(webview_page, "file://%s/bundle/index.html", cwd);

    webkit_web_view_load_uri(webview, webview_page);


    // Make sure that when the browser area becomes visible, it will get mouse and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(webview));

    // Make sure the main window and all its contents are visible
    gtk_widget_show_all(main_window);

    // Run the main GTK+ event loop
    gtk_main();

    return 0;
}

static void destroy_window_callback(GtkWidget* widget, GtkWidget* window) {
    gtk_main_quit();
}

static gboolean close_webview_callback(WebKitWebView* Webview, GtkWidget* window) {
    gtk_widget_destroy(window);
    return true;
}




JSObjectRef WebAppInterface_callAsConstructor(
    JSContextRef context,
    JSObjectRef constructor,
    size_t argumentCount,
    const JSValueRef arguments[],
    JSValueRef* exception
) {

}

void WebAppInterface_finalize(JSObjectRef object) {

}

JSValueRef WebAppInterface_openCollection(
    JSContextRef context,
    JSObjectRef function,
    JSObjectRef object,
    size_t argumentCount,
    const JSValueRef arguments[],
    JSValueRef* exception
) {
    JSStringRef response = JSStringCreateWithUTF8CString("{\"ok\":123}");

    return JSValueMakeString(context, response);
}

JSClassRef WebAppInterface () {

    JSClassDefinition classDefinition = kJSClassDefinitionEmpty;

    static JSStaticFunction staticFunctions[] = {
        { "openCollection", WebAppInterface_openCollection, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
        // { "saveCollection", WebAppInterface_saveCollection, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
        // { "getPoster", WebAppInterface_getPoster, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
        // { "savePoster", WebAppInterface_savePoster, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
        { 0, 0, 0 }
    };

    classDefinition.className = "WebkitgtkInterface";
    classDefinition.attributes = kJSClassAttributeNone;
    classDefinition.staticFunctions = staticFunctions;
    // classDefinition.staticValues = staticValues;
    classDefinition.finalize = WebAppInterface_finalize;
    // classDefinition.callAsConstructor = WebAppInterface_callAsConstructor;

    JSClassRef bridge_class = JSClassCreate(&classDefinition);

    return bridge_class;
}




/*


static JSValueRef bridge_open_collection(
    JSContextRef context,
    JSObjectRef function,
    JSObjectRef thisObject,
    size_t argumentCount,
    const JSValueRef arguments[],
    JSValueRef *exception
) {

}

static const JSStaticFunction bridge_staticfuncs[] =
{
    { "openCollection", bridge_open_collection, kJSPropertyAttributeReadOnly },
    // { "saveCollection", battery_percentage_cb, kJSPropertyAttributeReadOnly },
    // { "getPoster", battery_voltage_cb, kJSPropertyAttributeReadOnly },
    // { "savePoster", battery_update_time_cb, kJSPropertyAttributeReadOnly },
    { NULL, NULL, 0 }
};

static void battery_init_cb(JSContextRef context, JSObjectRef object) {

}

static const JSClassDefinition webview_bridge_def = {
    0,                      // version
    kJSClassAttributeNone,  // attributes
    "WebkitgtkInterface",   // className
    NULL,                   // parentClass
    NULL,                   // staticValues
    bridge_staticfuncs,     // staticFunctions
    battery_init_cb,        // initialize
    battery_destroy_cb,     // finalize
    NULL,                   // hasProperty
    NULL,                   // getProperty
    NULL,                   // setProperty
    NULL,                   // deleteProperty
    NULL,                   // getPropertyNames
    NULL,                   // callAsFunction
    NULL,                   // callAsConstructor
    NULL,                   // hasInstance  
    NULL,                   // convertToType
    NULL,                   // cachedPrototype
};



*/