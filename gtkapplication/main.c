
#include <config.h>
#include "application.h"


int main(int argc, char **argv) {

    #if PACKAGE_DEVELOPER_MODE
        g_message("DEV MODE");
        // inspector debug
        putenv("GTK_DEBUG=fatal-warnings");
        putenv("GOBJECT_DEBUG=instance-count");
        // putenv("G_ENABLE_DIAGNOSTIC=1");
    #endif

    int status;

    MovieApplication *app = movie_application_new(
        PACKAGE_APPLICATION_ID,
        G_APPLICATION_FLAGS_NONE //| G_APPLICATION_HANDLES_COMMAND_LINE | G_APPLICATION_HANDLES_OPEN
    );

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}
