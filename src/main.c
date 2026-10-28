#include <gtk/gtk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_N 3

typedef struct {
    GtkWidget *entries[MAX_N][MAX_N + 1];
    GtkWidget *equation_labels[MAX_N];
    GtkWidget *variable_headers[MAX_N];
    GtkWidget *dimension_dropdown;
    GtkWidget *grid;
    GtkWidget *result_header;
    GtkWidget *result_view;
    GtkWidget *status_label;
} AppWidgets;

static int selected_dimension(AppWidgets *w) {
    return gtk_drop_down_get_selected(GTK_DROP_DOWN(w->dimension_dropdown)) == 0 ? 2 : 3;
}

static double determinant(const double m[MAX_N][MAX_N], int n) {
    if (n == 2)
        return m[0][0] * m[1][1] - m[0][1] * m[1][0];
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

static void set_result(AppWidgets *w, const char *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w->result_view));
    gtk_text_buffer_set_text(buffer, text, -1);
}

static gboolean read_system(AppWidgets *w, double a[MAX_N][MAX_N], double b[MAX_N], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n + 1; j++) {
            GtkWidget *entry = j == n ? w->entries[i][MAX_N] : w->entries[i][j];
            const char *raw = gtk_editable_get_text(GTK_EDITABLE(entry));
            char *end = NULL;
            double value = g_ascii_strtod(raw, &end);
            while (end && g_ascii_isspace(*end)) end++;
            if (raw[0] == '\0' || end == raw || (end && *end != '\0') || !isfinite(value)) {
                char message[160];
                g_snprintf(message, sizeof(message), "Revisa la ecuación %d, columna %d: introduce un número válido.", i + 1, j + 1);
                gtk_label_set_text(GTK_LABEL(w->status_label), message);
                return FALSE;
            }
            if (j < n) a[i][j] = value;
            else b[i] = value;
        }
    }
    return TRUE;
}

static void append_matrix(GString *out, const char *name, const double m[MAX_N][MAX_N], int n) {
    g_string_append_printf(out, "%s =\n", name);
    for (int i = 0; i < n; i++) {
        g_string_append(out, "  [");
        for (int j = 0; j < n; j++)
            g_string_append_printf(out, " %10.4g", m[i][j]);
        g_string_append(out, " ]\n");
    }
    g_string_append_c(out, '\n');
}

static void solve_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppWidgets *w = user_data;
    int n = selected_dimension(w);
    double a[MAX_N][MAX_N] = {{0}}, b[MAX_N] = {0};
    if (!read_system(w, a, b, n)) return;

    double replaced[MAX_N][MAX_N][MAX_N] = {{{0}}};
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++)
                replaced[k][i][j] = (j == k) ? b[i] : a[i][j];
        }
    }

    double d = determinant(a, n);
    double deltas[MAX_N] = {0};
    for (int k = 0; k < n; k++)
        deltas[k] = determinant(replaced[k], n);

    GString *out = g_string_new(n == 2 ? "KRAMERLAB 2x2  |  PROCEDIMIENTO\n"
                                       : "KRAMERLAB 3x3  |  PROCEDIMIENTO\n");
    g_string_append(out, "==================================================\n\n");
    append_matrix(out, "Matriz de coeficientes A", a, n);
    const char *variables[] = {"x", "y", "z"};
    for (int k = 0; k < n; k++) {
        char name[96];
        g_snprintf(name, sizeof(name), "Matriz Δ%s (%s columna reemplazada por términos independientes)",
                   variables[k], k == 0 ? "primera" : k == 1 ? "segunda" : "tercera");
        append_matrix(out, name, replaced[k], n);
    }
    if (n == 2)
        g_string_append(out, "Determinantes:\nΔ = a·d − b·c\n");
    else
        g_string_append(out, "Determinante por expansión de la primera fila:\n"
                             "Δ = a(ei − fh) − b(di − fg) + c(dh − eg)\n");
    g_string_append_printf(out, "Δ = %.8g\n", d);
    for (int k = 0; k < n; k++)
        g_string_append_printf(out, "Δ%s = %.8g\n", variables[k], deltas[k]);
    g_string_append_c(out, '\n');

    if (fabs(d) < 1e-10) {
        g_string_append(out, "No existe una solución única porque Δ = 0.\n");
        g_string_append(out, "El sistema puede ser incompatible o tener infinitas soluciones; se necesita analizar sus rangos para distinguirlos.");
        gtk_label_set_text(GTK_LABEL(w->status_label), "Sistema singular: el determinante principal es cero.");
    } else {
        g_string_append(out, "Aplicando la regla de Cramer:\n");
        for (int k = 0; k < n; k++)
            g_string_append_printf(out, "%s = Δ%s / Δ = %.8g / %.8g = %.8g\n",
                                   variables[k], variables[k], deltas[k], d, deltas[k] / d);
        g_string_append(out, "\nSOLUCIÓN\n");
        for (int k = 0; k < n; k++)
            g_string_append_printf(out, "  %s = %.8g\n", variables[k], deltas[k] / d);
        gtk_label_set_text(GTK_LABEL(w->status_label), "Sistema resuelto correctamente.");
    }
    set_result(w, out->str);
    g_string_free(out, TRUE);
}

static void load_example_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppWidgets *w = user_data;
    int n = selected_dimension(w);
    const char *example_2x2[2][3] = {{"1", "1", "5"}, {"2", "-1", "1"}};
    const char *example_3x3[3][4] = {
        {"2", "1", "-1", "8"},
        {"-3", "-1", "2", "-11"},
        {"-2", "1", "2", "-3"}
    };
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n + 1; j++)
            gtk_editable_set_text(GTK_EDITABLE(j == n ? w->entries[i][MAX_N] : w->entries[i][j]),
                                  n == 2 ? example_2x2[i][j] : example_3x3[i][j]);
    gtk_label_set_text(GTK_LABEL(w->status_label), "Ejemplo cargado. Pulsa Resolver para ver el procedimiento.");
}

static void clear_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppWidgets *w = user_data;
    for (int i = 0; i < MAX_N; i++)
        for (int j = 0; j < MAX_N + 1; j++)
            gtk_editable_set_text(GTK_EDITABLE(w->entries[i][j]), "");
    set_result(w, "Introduce los coeficientes y términos independientes, después pulsa Resolver.\n\nEl procedimiento mostrará las matrices y los determinantes del sistema seleccionado.");
    gtk_label_set_text(GTK_LABEL(w->status_label), "Listo para un nuevo sistema.");
}

static GtkWidget *make_card(const char *title, GtkWidget **content_out) {
    GtkWidget *frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_add_css_class(frame, "card");
    GtkWidget *heading = gtk_label_new(title);
    gtk_widget_add_css_class(heading, "section-title");
    gtk_label_set_xalign(GTK_LABEL(heading), 0);
    gtk_box_append(GTK_BOX(frame), heading);
    *content_out = frame;
    return frame;
}

static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    const char *css =
        "window { background: #0b1020; color: #e8edf8; }"
        ".main-wrap { padding: 26px; }"
        ".hero { background: linear-gradient(135deg, #162447, #122d38); border: 1px solid #2c4963; border-radius: 18px; padding: 22px; }"
        ".hero-title { font-size: 27px; font-weight: 800; color: #f4f8ff; }"
        ".hero-sub { color: #a8bad2; font-size: 13px; }"
        ".card { background: #121a2d; border: 1px solid #27344d; border-radius: 16px; padding: 18px; }"
        ".section-title { color: #7de3d0; font-size: 16px; font-weight: 700; }"
        ".matrix-head { color: #9aabc4; font-size: 12px; font-weight: 700; }"
        "entry { background: #0b1222; color: #f2f6ff; border: 1px solid #33425e; border-radius: 8px; min-height: 34px; min-width: 55px; padding: 4px 7px; }"
        "entry:focus { border-color: #50d8c4; }"
        "button { background: #233451; color: #eaf1ff; border: 1px solid #3b5072; border-radius: 9px; padding: 9px 14px; font-weight: 700; }"
        "button.suggested-action { background: #24b9a7; color: #061a20; border-color: #24b9a7; }"
        "textview { background: #090f1d; color: #dce8fa; font-family: monospace; font-size: 12px; }"
        ".status { color: #91a4c1; font-size: 12px; }";
    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void dimension_changed(GObject *dropdown, GParamSpec *property, gpointer user_data) {
    (void)dropdown;
    (void)property;
    AppWidgets *w = user_data;
    int n = selected_dimension(w);

    gtk_widget_set_visible(w->variable_headers[2], n == 3);
    g_object_ref(w->result_header);
    gtk_grid_remove(GTK_GRID(w->grid), w->result_header);
    gtk_grid_attach(GTK_GRID(w->grid), w->result_header, n + 1, 0, 1, 1);
    g_object_unref(w->result_header);
    for (int i = 0; i < MAX_N; i++) {
        gtk_widget_set_visible(w->equation_labels[i], i < n);
        for (int j = 0; j < MAX_N; j++)
            gtk_widget_set_visible(w->entries[i][j], i < n && j < n);
        g_object_ref(w->entries[i][MAX_N]);
        gtk_grid_remove(GTK_GRID(w->grid), w->entries[i][MAX_N]);
        gtk_grid_attach(GTK_GRID(w->grid), w->entries[i][MAX_N], n + 1, i + 1, 1, 1);
        g_object_unref(w->entries[i][MAX_N]);
        gtk_widget_set_visible(w->entries[i][MAX_N], i < n);
    }
    gtk_label_set_text(GTK_LABEL(w->status_label),
                       n == 2 ? "Seleccionado: sistema de 2 ecuaciones y 2 incógnitas."
                              : "Seleccionado: sistema de 3 ecuaciones y 3 incógnitas.");
}

static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;
    load_css();
    AppWidgets *w = g_new0(AppWidgets, 1);

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "KramerLab — Regla de Cramer");
    gtk_window_set_default_size(GTK_WINDOW(window), 1040, 760);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_window_set_child(GTK_WINDOW(window), scroll);
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
    gtk_widget_add_css_class(root, "main-wrap");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), root);

    GtkWidget *hero = gtk_box_new(GTK_ORIENTATION_VERTICAL, 7);
    gtk_widget_add_css_class(hero, "hero");
    GtkWidget *title = gtk_label_new("KRAMERLAB");
    gtk_widget_add_css_class(title, "hero-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0);
    GtkWidget *subtitle = gtk_label_new("Calculadora de sistemas lineales · Regla de Cramer · C + GTK 4");
    gtk_widget_add_css_class(subtitle, "hero-sub");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0);
    gtk_box_append(GTK_BOX(hero), title);
    gtk_box_append(GTK_BOX(hero), subtitle);
    gtk_box_append(GTK_BOX(root), hero);

    GtkWidget *columns = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 18);
    gtk_widget_set_hexpand(columns, TRUE);
    gtk_box_append(GTK_BOX(root), columns);

    GtkWidget *input_card;
    make_card("Sistema de ecuaciones", &input_card);
    gtk_widget_set_hexpand(input_card, TRUE);
    gtk_widget_set_valign(input_card, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(columns), input_card);

    GtkWidget *hint = gtk_label_new("Elige el tamaño del sistema e introduce sus coeficientes y términos independientes.");
    gtk_label_set_wrap(GTK_LABEL(hint), TRUE);
    gtk_label_set_xalign(GTK_LABEL(hint), 0);
    gtk_box_append(GTK_BOX(input_card), hint);

    GtkWidget *dimension_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *dimension_label = gtk_label_new("Tamaño del sistema:");
    gtk_widget_add_css_class(dimension_label, "matrix-head");
    gtk_box_append(GTK_BOX(dimension_row), dimension_label);
    const char *dimension_options[] = {"2x2 (2 incógnitas)", "3x3 (3 incógnitas)", NULL};
    GtkStringList *dimension_list = gtk_string_list_new(dimension_options);
    w->dimension_dropdown = gtk_drop_down_new(G_LIST_MODEL(dimension_list), NULL);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(w->dimension_dropdown), 1);
    gtk_box_append(GTK_BOX(dimension_row), w->dimension_dropdown);
    gtk_box_append(GTK_BOX(input_card), dimension_row);

    GtkWidget *grid = gtk_grid_new();
    w->grid = grid;
    gtk_grid_set_row_spacing(GTK_GRID(grid), 9);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 7);
    const char *headers[] = {"x", "y", "z", "Resultado"};
    GtkWidget *equation_header = gtk_label_new("Ecuación");
    gtk_widget_add_css_class(equation_header, "matrix-head");
    gtk_grid_attach(GTK_GRID(grid), equation_header, 0, 0, 1, 1);
    for (int j = 0; j < MAX_N; j++) {
        GtkWidget *label = gtk_label_new(headers[j]);
        gtk_widget_add_css_class(label, "matrix-head");
        w->variable_headers[j] = label;
        gtk_grid_attach(GTK_GRID(grid), label, j + 1, 0, 1, 1);
    }
    w->result_header = gtk_label_new(headers[MAX_N]);
    gtk_widget_add_css_class(w->result_header, "matrix-head");
    gtk_grid_attach(GTK_GRID(grid), w->result_header, MAX_N + 1, 0, 1, 1);
    for (int i = 0; i < MAX_N; i++) {
        char row_label[24];
        g_snprintf(row_label, sizeof(row_label), "Ecuación %d", i + 1);
        GtkWidget *label = gtk_label_new(row_label);
        gtk_label_set_xalign(GTK_LABEL(label), 0);
        w->equation_labels[i] = label;
        gtk_grid_attach(GTK_GRID(grid), label, 0, i + 1, 1, 1);
        for (int j = 0; j < MAX_N; j++) {
            GtkWidget *entry = gtk_entry_new();
            gtk_widget_set_size_request(entry, 70, -1);
            gtk_entry_set_input_purpose(GTK_ENTRY(entry), GTK_INPUT_PURPOSE_NUMBER);
            gtk_widget_set_hexpand(entry, TRUE);
            w->entries[i][j] = entry;
            gtk_grid_attach(GTK_GRID(grid), entry, j + 1, i + 1, 1, 1);
        }
        GtkWidget *result_entry = gtk_entry_new();
        gtk_widget_set_size_request(result_entry, 70, -1);
        gtk_entry_set_input_purpose(GTK_ENTRY(result_entry), GTK_INPUT_PURPOSE_NUMBER);
        gtk_widget_set_hexpand(result_entry, TRUE);
        w->entries[i][MAX_N] = result_entry;
        gtk_grid_attach(GTK_GRID(grid), result_entry, MAX_N + 1, i + 1, 1, 1);
    }
    g_signal_connect(w->dimension_dropdown, "notify::selected", G_CALLBACK(dimension_changed), w);
    gtk_box_append(GTK_BOX(input_card), grid);

    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *solve = gtk_button_new_with_label("Resolver sistema");
    gtk_widget_add_css_class(solve, "suggested-action");
    GtkWidget *example = gtk_button_new_with_label("Cargar ejemplo");
    GtkWidget *clear = gtk_button_new_with_label("Limpiar");
    gtk_box_append(GTK_BOX(buttons), solve);
    gtk_box_append(GTK_BOX(buttons), example);
    gtk_box_append(GTK_BOX(buttons), clear);
    gtk_box_append(GTK_BOX(input_card), buttons);

    GtkWidget *result_card;
    make_card("Procedimiento y resultado", &result_card);
    gtk_widget_set_hexpand(result_card, TRUE);
    gtk_widget_set_vexpand(result_card, TRUE);
    gtk_box_append(GTK_BOX(columns), result_card);
    GtkWidget *result_scroll = gtk_scrolled_window_new();
    gtk_widget_set_size_request(result_scroll, 420, 390);
    gtk_widget_set_vexpand(result_scroll, TRUE);
    w->result_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(w->result_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(w->result_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(w->result_view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(w->result_view), GTK_WRAP_NONE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(result_scroll), w->result_view);
    gtk_box_append(GTK_BOX(result_card), result_scroll);

    w->status_label = gtk_label_new("Listo. Puedes cargar el ejemplo o introducir tu propio sistema.");
    gtk_widget_add_css_class(w->status_label, "status");
    gtk_label_set_xalign(GTK_LABEL(w->status_label), 0);
    gtk_box_append(GTK_BOX(root), w->status_label);
    dimension_changed(G_OBJECT(w->dimension_dropdown), NULL, w);

    g_signal_connect(solve, "clicked", G_CALLBACK(solve_clicked), w);
    g_signal_connect(example, "clicked", G_CALLBACK(load_example_clicked), w);
    g_signal_connect(clear, "clicked", G_CALLBACK(clear_clicked), w);

    set_result(w, "Elige un sistema de 2x2 o 3x3.\n\nPulsa ‘Cargar ejemplo’ para probar la calculadora.");
    load_example_clicked(GTK_BUTTON(example), w);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.kramerlab.calculadora3x3", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
