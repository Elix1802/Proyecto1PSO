#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data) {
    // Cargar el CSS personalizado
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, "Style.css");
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Cargar la interfaz
    GtkBuilder *builder = gtk_builder_new_from_file("compressor.ui");
    GObject *window = gtk_builder_get_object(builder, "windowMain");

    if (!window) {
        g_printerr("No se encontro el objeto 'main_window' en compressor.ui\n");
        return;
    }

    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_window_present(GTK_WINDOW(window));

    g_object_unref(builder);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "org.curso.compressor.preview",
        G_APPLICATION_DEFAULT_FLAGS
    );
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
