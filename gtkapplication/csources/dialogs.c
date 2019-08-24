#include "dialogs.h"
#include <stddef.h>


// simple message dialog

void dialog_message(GtkWindow *window, char *message, char *message2) {

    GtkWidget *dialog = gtk_message_dialog_new(window,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        message
    );
    if(message2 != NULL) {
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", message2);
    }

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

char *dialog_file_chooser(GtkWindow *window, char *existing) {
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    char *filename = NULL;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Ouvrir un fichier", window, action, // title
        "Annuler", GTK_RESPONSE_CANCEL,
        "Ouvrir", GTK_RESPONSE_ACCEPT,
    NULL);

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_mime_type(filter, "application/ndjson");
    gtk_file_filter_add_mime_type(filter, "application/json");
    gtk_file_filter_add_mime_type(filter, "text/plain");
    gtk_file_chooser_set_filter(chooser, filter);

    if(existing != NULL) { // file already exist
        gtk_file_chooser_set_filename(chooser, existing);
    }

    int status;
    if((status = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(chooser);
    }

    gtk_widget_destroy(dialog);

    return filename;
}

char *dialog_file_save(GtkWindow *window, char *existing) {
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    char *filename = NULL;

    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Ouvrir un fichier", window, action, // title
        "Annuler", GTK_RESPONSE_CANCEL,
        "Enregistrer", GTK_RESPONSE_ACCEPT,
    NULL);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_mime_type(filter, "application/ndjson");
    gtk_file_filter_add_mime_type(filter, "application/json");
    gtk_file_filter_add_mime_type(filter, "text/plain");
    gtk_file_chooser_set_filter(chooser, filter);

    if(existing != NULL) { // file already exist
        gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
        gtk_file_chooser_set_filename(chooser, existing);
    } else {
        gtk_file_chooser_set_current_name(chooser, "MyCollection.ndjson");
    }

    int status;
    if((status = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(chooser);
    }

    gtk_widget_destroy(dialog);

    return filename;
}
