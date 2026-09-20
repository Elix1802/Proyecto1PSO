#include <gtk/gtk.h>

static void on_folder_dialog_response(GObject *source, GAsyncResult *result, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    GtkLabel *label = GTK_LABEL(user_data);
    GError *error = NULL;

    GFile *folder = gtk_file_dialog_select_folder_finish(dialog, result, &error);
    if (folder) {
        char *path = g_file_get_path(folder);
        gtk_label_set_text(label, path);
        g_free(path);
        g_object_unref(folder);
    } else {
        if (error) {
            g_printerr("No se selecciono carpeta: %s\n", error->message);
            g_error_free(error);
        }
    }
}

static void on_open_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *label = GTK_WIDGET(user_data);
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));

    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Seleccionar directorio");
    gtk_file_dialog_select_folder(dialog, parent_window, NULL, on_folder_dialog_response, label);
}

static void on_compress_button_clicked(GtkButton *button, gpointer user_data) {
    ///Implementar logica de compresion
}

static void on_decompress_button_clicked(GtkButton *button, gpointer user_data) {
    ///Implementar logica de descompresion
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "Style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    GtkBuilder *builder = gtk_builder_new_from_file("compressor.ui");
    GObject *window = gtk_builder_get_object(builder, "windowMain");

    if (!window) {
        g_printerr("No se encontro el objeto 'windowMain' en compressor.ui\n");
        return;
    }

    GObject *dir_label = gtk_builder_get_object(builder, "lblDirectory");
    if (!dir_label) {
        g_printerr("No se encontro 'lblDirectory'\n");
    }

    GObject *open_button = gtk_builder_get_object(builder, "btnOpenFile");
    if (open_button) {
        g_signal_connect(open_button, "clicked", G_CALLBACK(on_open_button_clicked), dir_label);
    } else {
        g_printerr("No se encontro 'btnOpenFile'\n");
    }

    GObject *compress_button = gtk_builder_get_object(builder, "btnCompress");
    if (compress_button) {
        g_signal_connect(compress_button, "clicked", G_CALLBACK(on_compress_button_clicked), window);
    } else {
        g_printerr("No se encontro 'btnCompress'\n");
    }

    GObject *decompress_button = gtk_builder_get_object(builder, "btnDecompress");
    if (decompress_button) {
        g_signal_connect(decompress_button, "clicked", G_CALLBACK(on_decompress_button_clicked), window);
    } else {
        g_printerr("No se encontro 'btnDecompress'\n");
    }

    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_window_present(GTK_WINDOW(window));

    g_object_unref(builder);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "compressor.preview",
        G_APPLICATION_DEFAULT_FLAGS
    );
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
