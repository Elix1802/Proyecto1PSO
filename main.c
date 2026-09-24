#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include "algorithms.h"

static char *selected_directory = NULL;

#define METHOD_COUNT 3
char *css_path;
char *ui_path;

typedef struct {
    char method[32];
    char healthPercentage[10];
    double compress_time_s;
    double decompress_time_s;
    double compAcceleration;
    double decompAcceleration;
    double filesSize;
    double compressedSize;
    double radius;
} StatRecordIPC;

typedef struct {
    GtkWidget *loading_dialog;
    pid_t child_pid;
    guint pulse_source_id;
    int pipe_read_fd;
    int op_type;
} CompressionTaskData;

static StatRecordIPC g_compStats[METHOD_COUNT];
static StatRecordIPC g_decompStats[METHOD_COUNT];
static gboolean g_hasComp = FALSE;
static gboolean g_hasDecomp = FALSE;
static GtkWidget *grid_stats_widget = NULL;

static void render_stats_grid(void);

static void toIPC(StatRecordIPC *dst, StatRecord *src) {
    if (src == NULL) {
        memset(dst, 0, sizeof(StatRecordIPC));
        strncpy(dst->method, "N/A", sizeof(dst->method) - 1);
        strncpy(dst->healthPercentage, "0%", sizeof(dst->healthPercentage) - 1);
        return;
    }
    strncpy(dst->method, src->method, sizeof(dst->method) - 1);
    dst->method[sizeof(dst->method) - 1] = '\0';
    strncpy(dst->healthPercentage, src->healthPercentage, sizeof(dst->healthPercentage) - 1);
    dst->healthPercentage[sizeof(dst->healthPercentage) - 1] = '\0';
    dst->compress_time_s = src->compress_time_s;
    dst->decompress_time_s = src->decompress_time_s;
    dst->compAcceleration = src->compAcceleration;
    dst->decompAcceleration = src->decompAcceleration;
    dst->filesSize = src->filesSize;
    dst->compressedSize = src->compressedSize;
    dst->radius = src->radius;
}

static gboolean pulse_progress(gpointer user_data)
{
    GtkProgressBar *progress = GTK_PROGRESS_BAR(user_data);
    gtk_progress_bar_pulse(progress);
    return G_SOURCE_CONTINUE;
}

static void clear_stats_grid(GtkGrid *grid) {
    int row = 1;
    GtkWidget *child;
    while ((child = gtk_grid_get_child_at(grid, 0, row)) != NULL) {
        for (int col = 0; col < 9; col++) {
            GtkWidget *cell = gtk_grid_get_child_at(grid, col, row);
            if (cell) gtk_grid_remove(grid, cell);
        }
        row++;
    }
}

static void add_stat_row(GtkGrid *grid, int row, const StatRecord *stat) {
    char str_comp[32], str_decomp[32], str_comp_accel[32];
    char str_decomp_accel[32], str_fsize[32], str_csize[32], str_rad[32];

    snprintf(str_comp, sizeof(str_comp), "%.4f s", stat->compress_time_s);
    snprintf(str_decomp, sizeof(str_decomp), "%.4f s", stat->decompress_time_s);
    snprintf(str_comp_accel, sizeof(str_comp_accel), "%.2f %%", stat->compAcceleration);
    snprintf(str_decomp_accel, sizeof(str_decomp_accel), "%.2f %%", stat->decompAcceleration);
    snprintf(str_fsize, sizeof(str_fsize), "%.2f KB", stat->filesSize);
    snprintf(str_csize, sizeof(str_csize), "%.2f KB", stat->compressedSize);
    snprintf(str_rad, sizeof(str_rad), "%.2f %%", stat->radius);

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

static void render_stats_grid(void) {
    if (grid_stats_widget == NULL) return;

    static const char *methodNames[METHOD_COUNT] = {"Basic", "Threads", "Fork"};
    StatRecord merged[METHOD_COUNT];

    double basicCompTime = g_hasComp ? g_compStats[0].compress_time_s : 0.0;
    double basicDecompTime = g_hasDecomp ? g_decompStats[0].decompress_time_s : 0.0;

    for (int i = 0; i < METHOD_COUNT; i++) {
        memset(&merged[i], 0, sizeof(merged[i]));
        const char *method = methodNames[i];
        if (g_hasComp && g_compStats[i].method[0] != '\0') method = g_compStats[i].method;
        else if (g_hasDecomp && g_decompStats[i].method[0] != '\0') method = g_decompStats[i].method;

        merged[i].method = (char *)method;
        const char *healthSrc = g_hasDecomp ? g_decompStats[i].healthPercentage : (g_hasComp ? g_compStats[i].healthPercentage : "0%");
        strncpy(merged[i].healthPercentage, healthSrc, sizeof(merged[i].healthPercentage) - 1);
        merged[i].healthPercentage[sizeof(merged[i].healthPercentage) - 1] = '\0';
        
        merged[i].compress_time_s = g_hasComp ? g_compStats[i].compress_time_s : 0.0;
        merged[i].decompress_time_s = g_hasDecomp ? g_decompStats[i].decompress_time_s : 0.0;
        merged[i].filesSize = g_hasComp ? g_compStats[i].filesSize : (g_hasDecomp ? g_decompStats[i].filesSize : 0.0);
        merged[i].compressedSize = g_hasComp ? g_compStats[i].compressedSize : (g_hasDecomp ? g_decompStats[i].compressedSize : 0.0);
        merged[i].radius = g_hasComp ? g_compStats[i].radius
                 : (g_hasDecomp ? g_decompStats[i].radius : 0.0);

        merged[i].compAcceleration = (g_hasComp && basicCompTime > 0.0 && merged[i].compress_time_s > 0.0)
            ? (basicCompTime / merged[i].compress_time_s) * 100.0
            : 0.0;

        merged[i].decompAcceleration = (g_hasDecomp && basicDecompTime > 0.0 && merged[i].decompress_time_s > 0.0)
            ? (basicDecompTime / merged[i].decompress_time_s) * 100.0
            : 0.0;
    }

    clear_stats_grid(GTK_GRID(grid_stats_widget));
    populate_stats_grid(GTK_GRID(grid_stats_widget), merged, METHOD_COUNT);
}

static void on_child_finished(GPid pid, gint status, gpointer user_data)
{
    CompressionTaskData *task = (CompressionTaskData *)user_data;

    if (WIFEXITED(status)) {

    } else {
        g_printerr(
            "Proceso hijo [%d] terminó de forma no esperada.\n",
            pid
        );
    }

    StatRecordIPC ipcRecords[METHOD_COUNT];
    ssize_t totalRead = 0;
    ssize_t expected = sizeof(StatRecordIPC) * METHOD_COUNT;
    char *buf = (char *)ipcRecords;

    while (totalRead < expected) {
        ssize_t n = read(task->pipe_read_fd, buf + totalRead, expected - totalRead);
        if (n <= 0) break;
        totalRead += n;
    }
    close(task->pipe_read_fd);

    if (totalRead == expected) {
        if (task->op_type == 1) {
            memcpy(g_compStats, ipcRecords, sizeof(g_compStats));
            g_hasComp = TRUE;
        } else if (task->op_type == 2) {
            memcpy(g_decompStats, ipcRecords, sizeof(g_decompStats));
            g_hasDecomp = TRUE;
        }
        render_stats_grid();
    } else {
        g_printerr("No se pudieron leer las estadísticas del proceso hijo.\n");
    }

    if (task->pulse_source_id != 0) {
        g_source_remove(task->pulse_source_id);
        task->pulse_source_id = 0;
    }

    if (GTK_IS_WIDGET(task->loading_dialog)) {
        gtk_window_destroy(GTK_WINDOW(task->loading_dialog));
    }

    g_spawn_close_pid(pid);

    g_free(task);
}

static GtkWidget* create_loading_dialog(GtkWindow *parent, int mode)
{
    GtkWidget *dialog = gtk_window_new();

    gtk_window_set_title(GTK_WINDOW(dialog), "");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 280, 120);
    gtk_window_set_deletable(GTK_WINDOW(dialog), FALSE);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);

    GtkWidget *label;

    if (mode == 1) {
        label = gtk_label_new("Compressing files, please wait...");
    } else {
        label = gtk_label_new("Decompressing files, please wait...");
    }

    GtkWidget *progress = gtk_progress_bar_new();

    gtk_progress_bar_set_show_text(
        GTK_PROGRESS_BAR(progress),
        TRUE
    );

    gtk_progress_bar_set_text(
        GTK_PROGRESS_BAR(progress),
        "Processing..."
    );

    gtk_box_append(GTK_BOX(vbox), label);
    gtk_box_append(GTK_BOX(vbox), progress);

    gtk_window_set_child(GTK_WINDOW(dialog), vbox);

    return dialog;
}

static void on_folder_dialog_response(GObject *source, GAsyncResult *result, gpointer user_data) {
    GtkFileDialog *dialog = GTK_FILE_DIALOG(source);
    GtkLabel *label = GTK_LABEL(user_data);
    GError *error = NULL;

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

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        g_printerr("Error al crear el pipe para comunicación entre procesos.\n");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        g_printerr("Error al crear el proceso hijo con fork()\n");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) {
        close(pipefd[0]);
        StatRecordIPC ipcRecord;

        
        StatRecord *r1 = compressAllFiles(selected_directory);
        toIPC(&ipcRecord, r1);
        write(pipefd[1], &ipcRecord, sizeof(StatRecordIPC));
        if (r1) free(r1);
        

        StatRecord *r2 = compressAllFilesThreads(selected_directory);
        toIPC(&ipcRecord, r2);
        write(pipefd[1], &ipcRecord, sizeof(StatRecordIPC));
        if (r2) free(r2);
        

        
        StatRecord *r3 = compressAllFilesFork(selected_directory);
        toIPC(&ipcRecord, r3);
        write(pipefd[1], &ipcRecord, sizeof(StatRecordIPC));
        if (r3) free(r3);


        close(pipefd[1]);
        _exit(0);
    } else {
        close(pipefd[1]);

        GtkWidget *loading_dialog = create_loading_dialog(parent_window, 1);

        CompressionTaskData *task = g_new(CompressionTaskData, 1);

        task->loading_dialog = loading_dialog;
        task->child_pid = pid;
        task->pipe_read_fd = pipefd[0];
        task->op_type = 1;

        GtkWidget *content = gtk_window_get_child(
            GTK_WINDOW(loading_dialog)
        );

        GtkWidget *progress = gtk_widget_get_last_child(content);

        task->pulse_source_id = g_timeout_add(
            100,
            pulse_progress,
            progress
        );

        gtk_window_present(GTK_WINDOW(loading_dialog));

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

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        g_printerr("Error al crear el pipe\n");
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        g_printerr("Error al crear el proceso hijo con fork()\n");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) {
        close(pipefd[0]);
        StatRecordIPC ipcRecord;

        
        StatRecord *r1 = decompressAllFiles(selected_directory);
        toIPC(&ipcRecord, r1);
        write(pipefd[1], &ipcRecord, sizeof(StatRecordIPC));
        if (r1) free(r1);



        StatRecord *r2 = decompressAllFilesThread(selected_directory);
        toIPC(&ipcRecord, r2);
        write(pipefd[1], &ipcRecord, sizeof(StatRecordIPC));
        if (r2) free(r2);
        

        
        StatRecord *r3 = decompressAllFilesFork(selected_directory);
        toIPC(&ipcRecord, r3);
        write(pipefd[1], &ipcRecord, sizeof(StatRecordIPC));
        if (r3) free(r3);
        

        close(pipefd[1]);
        _exit(0);

    } else {
        close(pipefd[1]);

        GtkWidget *loading_dialog = create_loading_dialog(parent_window, 2);

        CompressionTaskData *task = g_new(CompressionTaskData, 1);

        task->loading_dialog = loading_dialog;
        task->child_pid = pid;
        task->pipe_read_fd = pipefd[0];
        task->op_type = 2;

        GtkWidget *content = gtk_window_get_child(
            GTK_WINDOW(loading_dialog)
        );

        GtkWidget *progress = gtk_widget_get_last_child(content);

        task->pulse_source_id = g_timeout_add(
            100,
            pulse_progress,
            progress
        );

        gtk_window_present(GTK_WINDOW(loading_dialog));

        g_child_watch_add(pid, on_child_finished, task);
    }
}

static void activate(GtkApplication *app) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider, css_path);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    GtkBuilder *builder = gtk_builder_new_from_file(ui_path);
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
        grid_stats_widget = GTK_WIDGET(grid_stats);

        for (int col = 0; col < 9; col++) {
            GtkWidget *header_label = gtk_grid_get_child_at(GTK_GRID(grid_stats), col, 0);
            if (header_label) {
                gtk_widget_add_css_class(header_label, "header");
                gtk_widget_set_halign(header_label, GTK_ALIGN_FILL);
                gtk_widget_set_valign(header_label, GTK_ALIGN_FILL);
            }
        }
    } else {
        g_printerr("No se encontro 'gridStats'\n");
    }

    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_window_present(GTK_WINDOW(window));

    g_object_unref(builder);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
        "com.groupc.Compressor",
        G_APPLICATION_DEFAULT_FLAGS
    );

    if (g_file_test("/app/share/compressor/Style.css", G_FILE_TEST_EXISTS) &&
    g_file_test("/app/share/compressor/compressor.ui", G_FILE_TEST_EXISTS)) {

    css_path = "/app/share/compressor/Style.css";
    ui_path = "/app/share/compressor/compressor.ui";

    } else {

        css_path = "Style.css";
        ui_path = "compressor.ui";
    }

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    if (selected_directory != NULL) {
        g_free(selected_directory);
    }

    g_object_unref(app);
    return status;
}