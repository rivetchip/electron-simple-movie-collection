#pragma once

#include <glib.h>
#include <gtk/gtk.h>

#ifdef  __cplusplus
    extern "C" {
#endif


// public functions
void dialog_message(GtkWindow *window, char *message, char *message2);
char *dialog_file_chooser(GtkWindow *window, char *existing);
char *dialog_file_save(GtkWindow *window, char *existing);



#ifdef  __cplusplus
    } // extern c
#endif
