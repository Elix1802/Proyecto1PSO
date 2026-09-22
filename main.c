#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "algorithms.h"

static char *selected_directory = NULL;

typedef struct {
    GtkWidget *loading_dialog;
    pid_t child_pid;
} CompressionTaskData;

static void on_child_finished(GPid pid, gint status, gpointer user_data) {
    CompressionTaskData *task = (CompressionTaskData *)user_data;

    if (WIFEXITED(status)) {
        g_print("Proceso hijo [%d] finalizó con código: %d\n", pid, WEXITSTATUS(status));
    } else {
        g_printerr("Proceso hijo [%d] terminó de forma no esperada.\n", pid);
    }

    if (GTK_IS_WIDGET(task->loading_dialog)) {
        gtk_window_destroy(GTK_WINDOW(task->loading_dialog));
    }

    g_spawn_close_pid(pid);
    g_free(task);
}

static GtkWidget* create_loading_dialog(GtkWindow *parent, int mode) {
    GtkWidget *dialog = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 280, 120);
    gtk_window_set_deletable(GTK_WINDOW(dialog), FALSE); // Evita que la cierren manualmente

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    
    GtkWidget *label;
    if (mode == 1) label = gtk_label_new("Compressing files, please wait...");
    else label = gtk_label_new("Uncompressing files, please wait...");

    GtkWidget *spinner = gtk_spinner_new();
    gtk_spinner_start(GTK_SPINNER(spinner));

    gtk_box_append(GTK_BOX(vbox), label);
    gtk_box_append(GTK_BOX(vbox), spinner);

    gtk_window_set_child(GTK_WINDOW(dialog), vbox);
    return dialog;
}



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
        GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));

        GtkAlertDialog *alert = gtk_alert_dialog_new("%s", "⚠️ No directory selected for compression.");
        gtk_alert_dialog_show(alert, parent_window);
        g_object_unref(alert);


 
            return;

    }

    GtkWindow *parent_window = GTK_WINDOW(user_data);

    pid_t pid = fork();

    if (pid < 0) {
        g_printerr("Error al crear el proceso hijo con fork()\n");
        return;
    }

    if (pid == 0) {

        g_print("[Hijo %d] Iniciando compresión en: %s\n", getpid(), selected_directory);
        
        compressAllFilesFork(selected_directory);

        g_print("[Hijo %d] Compresión completada.\n", getpid());
        
        _exit(0);
    } else {

        g_print("[Padre] Hijo lanzado con PID: %d. Mostrando spinner...\n", pid);

        GtkWidget *loading_dialog = create_loading_dialog(parent_window, 1);
        gtk_window_present(GTK_WINDOW(loading_dialog));

        CompressionTaskData *task = g_new(CompressionTaskData, 1);
        task->loading_dialog = loading_dialog;
        task->child_pid = pid;

        g_child_watch_add(pid, on_child_finished, task);
    }
}

static void on_decompress_button_clicked(GtkButton *button, gpointer user_data) {
    if (selected_directory == NULL) {
        GtkWindow *parent_window = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(button)));

        GtkAlertDialog *alert = gtk_alert_dialog_new("%s", "⚠️ No directory selected for decompression.");
        gtk_alert_dialog_show(alert, parent_window);
        g_object_unref(alert);

        return;
    }
    GtkWindow *parent_window = GTK_WINDOW(user_data);

    pid_t pid = fork();

    if (pid < 0) {
        g_printerr("Error al crear el proceso hijo con fork()\n");
        return;
    }

    if (pid == 0) {

        g_print("[Hijo %d] Iniciando Descompresión en: %s\n", getpid(), selected_directory);
        
        //StatRecord* record = decompressAllFiles(selected_directory);
        decompressAllFilesFork(selected_directory);
        g_print("[Hijo %d] Descompresión completada.\n", getpid());

        //g_print("[Salud %s].\n", record->healthPercentage);
        
        _exit(0);

    } else {

        g_print("[Padre] Hijo lanzado con PID: %d. Mostrando spinner...\n", pid);

        GtkWidget *loading_dialog = create_loading_dialog(parent_window, 2);
        gtk_window_present(GTK_WINDOW(loading_dialog));

        CompressionTaskData *task = g_new(CompressionTaskData, 1);
        task->loading_dialog = loading_dialog;
        task->child_pid = pid;

        g_child_watch_add(pid, on_child_finished, task);
    }
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