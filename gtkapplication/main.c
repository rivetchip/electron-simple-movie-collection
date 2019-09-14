#include <config.h>
#include "application.h"
#include <locale.h>


int main(int argc, char **argv) {

    #if PACKAGE_DEVELOPER_MODE
        g_message("DEV MODE");
        // inspector debug
        putenv("GTK_DEBUG=fatal-warnings");
        putenv("GOBJECT_DEBUG=instance-count");
        putenv("G_ENABLE_DIAGNOSTIC=1");
    #endif

    int status;

    setlocale(LC_ALL, "");    


    MovieApplication *app = movie_application_new(
        PACKAGE_APPLICATION_ID, PACKAGE_BUILD_VERSION,
        G_APPLICATION_HANDLES_OPEN
    );

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}
