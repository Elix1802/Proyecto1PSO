#include <gtk/gtk.h>
#include "algorithms.h"

static char *selected_directory = NULL;



static void add_stat_row(GtkGrid *grid, int row, const StatRecord *stat) {
    char str_comp[32], str_decomp[32], str_comp_accel[32];
    char str_decomp_accel[32], str_fsize[32], str_csize[32], str_rad[32];

    snprintf(str_comp, sizeof(str_comp), "%.2f s", stat->compress_time_s);
    snprintf(str_decomp, sizeof(str_decomp), "%.2f s", stat->decompress_time_s);
    snprintf(str_comp_accel, sizeof(str_comp_accel), "%.2f %%", stat->compAcceleration);
    snprintf(str_decomp_accel, sizeof(str_decomp_accel), "%.2f %%", stat->decompAcceleration);
    snprintf(str_fsize, sizeof(str_fsize), "%.2f KB", stat->filesSize);
    snprintf(str_csize, sizeof(str_csize), "%.2f KB", stat->compressedSize);
    snprintf(str_rad, sizeof(str_rad), "%.2f mm", stat->radius);

    const char *values[] = {
        stat->method,
        stat->healthPercentage,
        str_comp,
        str_decomp,
        str_comp_accel,
        str_decomp_accel,
        str_fsize,
        str_csize,
        str_rad
    };

    for (int col = 0; col < 9; col++) {
        GtkWidget *label = gtk_label_new(values[col]);

        gtk_widget_set_halign(label, GTK_ALIGN_FILL);
        gtk_widget_set_valign(label, GTK_ALIGN_FILL);

        gtk_grid_attach(grid, label, col, row, 1, 1);
    }
}

static void populate_stats_grid(GtkGrid *grid, const StatRecord records[], size_t count) {
    for (size_t i = 0; i < count; i++) {
        add_stat_row(grid, (int)(i + 1), &records[i]);
    }
}

static void on_folder_dialog_response(GObject *source, GAsyncResult *result, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    GtkLabel *label = GTK_LABEL(user_data);
    GError *error = NULL;

    // Se utiliza select_folder_finish para obtener la carpeta seleccionada
    GFile *folder = gtk_file_dialog_select_folder_finish(dialog, result, &error);
    if (folder) {
        if (selected_directory != NULL) {
            g_free(selected_directory);
        }

        selected_directory = g_file_get_path(folder);
        gtk_label_set_text(label, selected_directory);

        g_object_unref(folder);
    } else {
        if (error) {
            if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
                g_printerr("Error al seleccionar carpeta: %s\n", error->message);
            }
            g_error_free(error);
        }
    }
}

static void on_open_button_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *label = GTK_WIDGET(user_data);
    GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));

    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Seleccionar directorio");
    
    // Se utiliza select_folder para abrir el explorador en modo seleccion de directorio
    gtk_file_dialog_select_folder(dialog, parent_window, NULL, on_folder_dialog_response, label);
}

static void on_compress_button_clicked(GtkButton *button, gpointer user_data) {
    if (selected_directory == NULL) {
        g_print("Atención: No hay una carpeta seleccionada para comprimir.\n");
        return;
    }
    compressAllFiles(selected_directory);
    g_print("Comprimiendo carpeta: %s\n", selected_directory);
}

static void on_decompress_button_clicked(GtkButton *button, gpointer user_data) {
    if (selected_directory == NULL) {
        g_print("Atención: No hay una carpeta seleccionada para descomprimir.\n");
        //return;
    }
    decompressAllFiles(selected_directory);
    g_print("Descomprimiendo carpeta: %s\n", selected_directory);
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
        gtk_widget_add_css_class(GTK_WIDGET(open_button), "btnOpenFile");
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

    GObject *grid_stats = gtk_builder_get_object(builder, "gridStats");
    if (grid_stats) {
        gtk_widget_add_css_class(GTK_WIDGET(grid_stats), "stats-grid");

        for (int col = 0; col < 9; col++) {
            GtkWidget *header_label = gtk_grid_get_child_at(GTK_GRID(grid_stats), col, 0);
            if (header_label) {
                gtk_widget_add_css_class(header_label, "header");
                gtk_widget_set_halign(header_label, GTK_ALIGN_FILL);
                gtk_widget_set_valign(header_label, GTK_ALIGN_FILL);
            }
        }

        StatRecord ejemploPrueba[] = {
            {"Huffman", "98%", 1.45, 0.32, 2.40, 3.10, 1024.0, 450.5, 12.50},
            {"Fork",    "95%", 0.88, 0.15, 3.10, 4.20, 2048.0, 810.0,  8.20},
            {"Pthread", "80%", 0.25, 0.08, 1.15, 1.30,  512.0, 400.0,  4.00}
        };

        size_t count = sizeof(ejemploPrueba) / sizeof(ejemploPrueba[0]);
        populate_stats_grid(GTK_GRID(grid_stats), ejemploPrueba, count);
    } else {
        g_printerr("No se encontro 'gridStats'\n");
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

    if (selected_directory != NULL) {
        g_free(selected_directory);
    }

    g_object_unref(app);
    return status;
}