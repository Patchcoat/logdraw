#include "gtkUI.h"

// Global variable! Living on the edge!
// This is assigned once, in UISetup, and then only read from after that point
// DO NOT write to outside of that function.
struct lflines {
    unsigned long int count;
    char** lines;
} loglines;
struct dtname {
    size_t count;
    char** names;
} datanames;

GtkWidget* new_mi(const char* label) {
    GtkWidget* newmi;
    newmi = gtk_menu_item_new_with_label(label);
    return newmi;
}

GtkWidget* labelWidget() {
    GtkWidget* labelBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* labelLabel = gtk_label_new("Label");
    GtkWidget* labelRow = gtk_list_box_row_new();
    GtkWidget* labelTxt = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)labelTxt, "Label Regex");
    gtk_box_pack_start(GTK_BOX(labelBox), labelLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(labelBox), labelTxt, FALSE, FALSE, 0);
    return labelBox;
}

GtkWidget* timeWidget() {
    GtkWidget* timeBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* timeLabel = gtk_label_new("Time");
    GtkWidget* timeTxt = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)timeTxt, "Timestamp Regex");
    GtkWidget* radioTimeBox = gtk_flow_box_new();
    GtkWidget* radioEpoch = gtk_radio_button_new_with_label(NULL,"Epoch");
    GtkWidget* radioDate = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioEpoch), "Date");
    GtkWidget* radioTime = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioEpoch), "Time");
    GtkWidget* radioDateTime = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioEpoch), "Date Time");
    gtk_box_pack_start(GTK_BOX(timeBox), timeLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(timeBox), timeTxt, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(timeBox), radioTimeBox, FALSE, FALSE, 0);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioTimeBox), radioEpoch, 0);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioTimeBox), radioDate, 1);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioTimeBox), radioTime, 2);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioTimeBox), radioDateTime, 3);
    return timeBox;
}

GtkWidget* dataWidget() {
    GtkWidget* dataBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* dataLabel = gtk_label_new("Data");
    GtkWidget* dataTxt = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)dataTxt, "Data Regex");
    GtkWidget* radioDataBox = gtk_flow_box_new();
    GtkWidget* radioInt = gtk_radio_button_new_with_label(NULL, "Int");
    GtkWidget* radioFloat = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioInt), "Float");
    GtkWidget* radioString = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioInt), "String");
    GtkWidget* radioVec2d = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioInt), "Vec2d");
    GtkWidget* radioVec3d = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioInt), "Vec3d");
    GtkWidget* radioVec4d = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioInt), "Vec4d");
    GtkWidget* radioColorRGB = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioInt), "ColorRGB");
    GtkWidget* radioColorHex = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioInt), "ColorHex");
    gtk_box_pack_start(GTK_BOX(dataBox), dataLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(dataBox), dataTxt, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(dataBox), radioDataBox, FALSE, FALSE, 0);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioDataBox), radioInt, 0);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioDataBox), radioFloat, 1);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioDataBox), radioString, 2);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioDataBox), radioVec2d, 3);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioDataBox), radioVec3d, 4);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioDataBox), radioVec4d, 5);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioDataBox), radioColorRGB, 6);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioDataBox), radioColorHex, 7);
    return dataBox;
}

void logScroll(GtkWidget* loglist) {
    GtkCssProvider* css = gtk_css_provider_new();
    GdkScreen* screen = gdk_display_get_default_screen(gdk_display_get_default());
    GtkStyleContext* context;

    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(css),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(css,
            ".data_btn {padding-top: 0px; padding-bottom: 0px;\
                        margin-bottom: -8px; font-family: monospace;}",
            -1, NULL);
    // TODO negative margins are bad and break when the theme changes.
    for (int i = 0; i < loglines.count; i++) {
        GtkWidget* btn = gtk_button_new();
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        char* label = loglines.lines[i];
        label[strlen(label)-1] = '\0';
        GtkWidget* text = gtk_label_new(label);

        context = gtk_widget_get_style_context(btn);
        gtk_style_context_add_class(context, "data_btn");

        gtk_box_pack_start(GTK_BOX(box), text, TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(btn), box);
        gtk_widget_set_halign(text, GTK_ALIGN_START);
        gtk_button_set_relief(GTK_BUTTON(btn), GTK_RELIEF_NONE);
        gtk_box_pack_start(GTK_BOX(loglist), btn, FALSE, FALSE, 0);
    }
    g_object_unref(css);
}

static void activate (GtkApplication* app, gpointer* user_data)
{
    GtkWidget* window = gtk_application_window_new(app);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);

    GtkWidget* menubar = gtk_menu_bar_new();
    GtkWidget* fileMenu = gtk_menu_new();
    GtkWidget* fileMi = gtk_menu_item_new_with_label("File");
    GtkWidget* quitMi = gtk_menu_item_new_with_label("Quit");
    GtkWidget* helpMi = gtk_menu_item_new_with_label("Help");

    GtkWidget* logbar = gtk_menu_bar_new();
    GtkWidget* logfileMi = new_mi("Logfile");
    gtk_menu_shell_append(GTK_MENU_SHELL(logbar), logfileMi);

    // Log Screen
    GtkWidget* logscroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget* loglist = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* logElmt = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* logatribscroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget* logatrib = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Log Scroll
    logScroll(loglist);

    GtkWidget* logMenu = gtk_menu_bar_new();
    GtkWidget* logAddMi = gtk_menu_item_new_with_label("+");
    for (int i = 0; i < datanames.count; i++) {
        // right screen menu
        GtkWidget* logMi = gtk_menu_item_new_with_label(datanames.names[i]);
        gtk_menu_shell_append(GTK_MENU_SHELL(logMenu), logMi);
        // top bar menu
        GtkWidget* logTopMi = new_mi(datanames.names[i]);
        gtk_menu_shell_append(GTK_MENU_SHELL(logbar), logTopMi);
    }

    // Log
    GtkWidget* logFrame = gtk_frame_new(NULL);
    GtkWidget* logLabel = gtk_label_new("Log String");

    // Window
    gtk_window_set_title(GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
    
    gtk_container_add(GTK_CONTAINER (window), vbox);

    // Menubar
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMi), fileMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu), quitMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileMi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), helpMi);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    g_signal_connect_swapped(quitMi, "activate", G_CALLBACK(gtk_widget_destroy), window);

    // Logfile Bar
    gtk_box_pack_start(GTK_BOX(vbox), logbar, FALSE, FALSE, 0);

    // Logfile Box
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);
    gtk_widget_set_size_request(paned, 200, -1);
    gtk_paned_pack1(GTK_PANED(paned), logscroll, TRUE, FALSE);
    gtk_widget_set_size_request(logscroll, 100, -1);
    gtk_container_add(GTK_CONTAINER(logscroll), loglist);
    gtk_paned_pack2(GTK_PANED(paned), logElmt, TRUE, FALSE);
    gtk_widget_set_size_request(logElmt, 50, -1);

    // Log Menu Bar
    // TODO add log#Mi programatically
    gtk_menu_shell_append(GTK_MENU_SHELL(logMenu), logAddMi);
    gtk_box_pack_start(GTK_BOX(logElmt), logMenu, FALSE, FALSE, 0);

    // Log Display
    gtk_box_pack_start(GTK_BOX(logElmt), logFrame, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(logFrame), logLabel);

    // Log Atributes
    gtk_box_pack_start(GTK_BOX(logElmt), logatribscroll, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(logatribscroll), logatrib);

    // Log Label
    gtk_box_pack_start(GTK_BOX(logatrib), labelWidget(), TRUE, TRUE, 0);

    // Log Time
    gtk_box_pack_start(GTK_BOX(logatrib), timeWidget(), TRUE, TRUE, 0);

    // Log Data
    gtk_box_pack_start(GTK_BOX(logatrib), dataWidget(), TRUE, TRUE, 0);

    gtk_widget_show_all (window);

    for (int i = 0; i < datanames.count; i++)
        free(datanames.names[i]);
    free(datanames.names);
}

GtkApplication* UISetup(char** lines, unsigned long int lncount, size_t grpcnt, grp* dtgrps) {
    loglines.lines = lines;
    loglines.count = lncount;
    datanames.count = grpcnt;
    datanames.names = (char**)malloc(grpcnt*sizeof(char*));
    for (int i = 0; i < grpcnt; i++) {
        datanames.names[i] = (char*)malloc(strlen(dtgrps[i].name)*sizeof(char*));
        strcpy(datanames.names[i], dtgrps[i].name);
    }
    GtkApplication* app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
    return app;
}

int UIStart(GtkApplication* app) {
    //int status = g_application_run (G_APPLICATION (app), argc, argv);
    int status = g_application_run (G_APPLICATION (app), 0, NULL);

    g_object_unref (app);
    return status;
}
