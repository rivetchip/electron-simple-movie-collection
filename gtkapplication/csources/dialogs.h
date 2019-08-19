#pragma once

#include <glib.h>
#include <gtk/gtk.h>

#ifdef  __cplusplus
    extern "C" {
#endif


// public api

void dialog_message(GtkWindow *window, char *message, char *message2);
char *dialog_file_chooser(char *existing);
char *show_save_dialog(char *existing);



#ifdef  __cplusplus
    } // extern c
#endif
