
#include <webkit2/webkit-web-extension.h>
#include <JavaScriptCore/JavaScript.h>

JSClassRef WebAppInterface();

static void window_object_cleared_callback(
    WebKitScriptWorld *world,
    WebKitWebPage *page,
    WebKitFrame *frame,
    gpointer user_data
) {
    g_message("extension:window_object_cleared_callback");
}

static void web_page_created_callback (
    WebKitWebExtension *extension,
    WebKitWebPage *web_page,
    gpointer user_data
) {
    g_message("extension:web_page_created_callback");
}


G_MODULE_EXPORT void webkit_web_extension_initialize (WebKitWebExtension *extension) {

    g_signal_connect(extension, "page-created", G_CALLBACK(web_page_created_callback), NULL);


    WebKitScriptWorld *webkit_world = webkit_script_world_get_default();

    g_signal_connect(webkit_world, "window-object-cleared", G_CALLBACK(window_object_cleared_callback), NULL);
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