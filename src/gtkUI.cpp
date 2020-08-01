#include "gtkUI.h"
#include "draw.h"
#include <string>
#include <pthread.h>

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

grp* uidtgrp;

GtkWidget* new_mi(const char* label) {
    GtkWidget* newmi;
    newmi = gtk_menu_item_new_with_label(label);
    return newmi;
}

GtkWidget* labelWidget(int index) {
    GtkWidget* labelBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* labelLabel = gtk_label_new("Label");
    GtkWidget* labelRow = gtk_list_box_row_new();
    GtkWidget* labelTxt = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)labelTxt, "Label Regex");
    gtk_box_pack_start(GTK_BOX(labelBox), labelLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(labelBox), labelTxt, FALSE, FALSE, 0);
    gtk_entry_set_text(GTK_ENTRY(labelTxt), uidtgrp[index].idstr);
    return labelBox;
}

GtkWidget* timeWidget(int index) {
    GtkWidget* timeBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* timeLabel = gtk_label_new("Time");
    GtkWidget* timeTxt = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)timeTxt, "Timestamp Regex");
    gtk_entry_set_text(GTK_ENTRY(timeTxt), uidtgrp[index].timestr);
    GtkWidget* radioTimeBox = gtk_flow_box_new();
    GtkWidget* radioEpoch = gtk_radio_button_new_with_label(NULL,"Epoch");
    GtkWidget* radioDate = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioEpoch), "Date");
    GtkWidget* radioTime = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioEpoch), "Time");
    GtkWidget* radioDateTime = gtk_radio_button_new_with_label_from_widget(
            GTK_RADIO_BUTTON(radioEpoch), "Date Time");
    GtkToggleButton* radioSelect; // the radio button to be selected by default
    switch (uidtgrp[index].tmfmt) {
      case Date:
        radioSelect = GTK_TOGGLE_BUTTON(radioDate);
        break;
      case Time:
        radioSelect = GTK_TOGGLE_BUTTON(radioTime);
        break;
      case DateTime:
        radioSelect = GTK_TOGGLE_BUTTON(radioDateTime);
        break;
      default: // default to epoch, meaning we don't need a case for Epoch
        radioSelect = GTK_TOGGLE_BUTTON(radioEpoch);
        break;
    }
    gtk_toggle_button_set_active(radioSelect, TRUE); // select that button
    gtk_box_pack_start(GTK_BOX(timeBox), timeLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(timeBox), timeTxt, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(timeBox), radioTimeBox, FALSE, FALSE, 0);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioTimeBox), radioEpoch, 0);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioTimeBox), radioDate, 1);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioTimeBox), radioTime, 2);
    gtk_flow_box_insert(GTK_FLOW_BOX(radioTimeBox), radioDateTime, 3);
    return timeBox;
}

GtkWidget* dataWidget(int index) {
    GtkWidget* dataBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* dataLabel = gtk_label_new("Data");
    GtkWidget* dataTxt = gtk_entry_new();
    gtk_entry_set_placeholder_text((GtkEntry*)dataTxt, "Data Regex");
    gtk_entry_set_text(GTK_ENTRY(dataTxt), uidtgrp[index].dtstr);
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
    GtkToggleButton* radioSelect; // the radio button to be selected by default
    switch (uidtgrp[index].dtfmt) {
      case Float:
        radioSelect = GTK_TOGGLE_BUTTON(radioFloat);
        break;
      case String:
        radioSelect = GTK_TOGGLE_BUTTON(radioString);
        break;
      case Vec2d:
        radioSelect = GTK_TOGGLE_BUTTON(radioVec2d);
        break;
      case Vec3d:
        radioSelect = GTK_TOGGLE_BUTTON(radioVec3d);
        break;
      case Vec4d:
        radioSelect = GTK_TOGGLE_BUTTON(radioVec4d);
        break;
      case ColorRGB:
        radioSelect = GTK_TOGGLE_BUTTON(radioColorRGB);
        break;
      case ColorHex:
        radioSelect = GTK_TOGGLE_BUTTON(radioColorHex);
        break;
      default: // default to int, meaning we don't need a case for Int
        radioSelect = GTK_TOGGLE_BUTTON(radioInt);
        break;
    }
    gtk_toggle_button_set_active(radioSelect, TRUE); // select that button
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
                        margin-bottom: 0px; font-family: monospace;}",
            -1, NULL);
    // TODO decrease the height of the logfile line buttons without using negative margins
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

void helpMenu(GtkWidget* widget, gpointer data) {
    printf("Help Function\n");
    // TODO open a help window
}

// display's the log, drawing it using an openGL window
void* logDisplay(void* indexPtr) {
    long index = (long)indexPtr;
    switch (uidtgrp[index].dtfmt) {
      case Int:
        printf("%d ", index);
        printf("Integer");
        break;
      case Float:
        printf("%d ", index);
        printf("Float");
        break;
      case String:
        printf("%d ", index);
        printf("String");
        break;
      case Vec2d:
        printf("%d ", index);
        printf("Vec2d");
        break;
      case Vec3d:
        drawDt3dLine(&uidtgrp[index]);
        break;
      case Vec4d:
        printf("%d ", index);
        printf("Vec4d");
        break;
      case ColorRGB:
        printf("%d ", index);
        printf("ColorRGB");
        break;
      case ColorHex:
        printf("%d ", index);
        printf("ColorHex");
        break;
      default:
        printf("%s\n", uidtgrp[index].name);
    }
    pthread_exit(NULL);
}

// creates a new thread to be used to display the log
void logDisplayThread(GtkWidget* widget, gpointer data) {
    long index = GPOINTER_TO_INT(data);
    pthread_t thread[1];
    int rc;
    rc = pthread_create(&thread[0], NULL, logDisplay, (void*)index);
}

// This is the standard window configuration
// it shows the log file contents
void standardView (GtkWidget* vbox) {
    GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    // Log Screen
    GtkWidget* logscroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget* loglist = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* logElmt = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Log Menu Bar
    GtkWidget* logbar = gtk_menu_bar_new();
    //GtkWidget* logfileMi = new_mi("Logfile");
    //gtk_menu_shell_append(GTK_MENU_SHELL(logbar), logfileMi);
    gtk_box_pack_start(GTK_BOX(vbox), logbar, FALSE, FALSE, 0);

    // Log Menu Bar Items
    //GtkWidget* logMenu = gtk_menu_bar_new();
    GtkWidget* logNotebook = gtk_notebook_new();
    for (int i = 0; i < datanames.count; i++) {
        // right screen menu
        //GtkWidget* logMi = gtk_menu_item_new_with_label(datanames.names[i]);
        //gtk_menu_shell_append(GTK_MENU_SHELL(logMenu), logMi);
        GtkWidget* logPage = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget* logAtribScroll = gtk_scrolled_window_new(NULL, NULL);
        GtkWidget* logAtribPage = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget* logNotebookLabel = gtk_label_new(datanames.names[i]);
        gtk_notebook_append_page(GTK_NOTEBOOK(logNotebook), logPage, logNotebookLabel);
        // Example Log
        GtkWidget* logFrame = gtk_frame_new(NULL);
        // TODO actually provide an example of a log, rather than "Log String"
        GtkWidget* logLabel = gtk_label_new("Log String");
        // Log Atribute Frame 
        gtk_box_pack_start(GTK_BOX(logPage), logFrame, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(logFrame), logLabel);
        // Log Atributes
        gtk_box_pack_start(GTK_BOX(logPage), logAtribScroll, TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(logAtribScroll), logAtribPage);
        // Log Label
        gtk_box_pack_start(GTK_BOX(logAtribPage), labelWidget(i), TRUE, TRUE, 0);
        // Log Time
        gtk_box_pack_start(GTK_BOX(logAtribPage), timeWidget(i), TRUE, TRUE, 0);
        // Log Data
        gtk_box_pack_start(GTK_BOX(logAtribPage), dataWidget(i), TRUE, TRUE, 0);

        // top bar menu
        GtkWidget* logTopMi = new_mi(datanames.names[i]);
        gtk_menu_shell_append(GTK_MENU_SHELL(logbar), logTopMi);
        g_signal_connect(logTopMi, "activate", G_CALLBACK(logDisplayThread), GINT_TO_POINTER(i));
    }

    // Log Notebook
    gtk_box_pack_start(GTK_BOX(logElmt), logNotebook, TRUE, TRUE, 0);

    // Log Scroll
    logScroll(loglist);

    // Logfile Box
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);
    gtk_widget_set_size_request(paned, 200, -1);
    gtk_paned_pack1(GTK_PANED(paned), logscroll, TRUE, FALSE);
    gtk_widget_set_size_request(logscroll, 100, -1);
    gtk_container_add(GTK_CONTAINER(logscroll), loglist);
    gtk_paned_pack2(GTK_PANED(paned), logElmt, TRUE, FALSE);
    gtk_widget_set_size_request(logElmt, 50, -1);

    for (int i = 0; i < datanames.count; i++)
        free(datanames.names[i]);
    free(datanames.names);

}

static void activate (GtkApplication* app, gpointer* user_data) {
    GtkWidget* window = gtk_application_window_new(app);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // top menu
    GtkWidget* menubar = gtk_menu_bar_new();
    GtkWidget* fileMenu = gtk_menu_new();
    GtkWidget* fileMi = gtk_menu_item_new_with_label("File");
    GtkWidget* quitMi = gtk_menu_item_new_with_label("Quit");
    GtkWidget* helpMi = gtk_menu_item_new_with_label("Help");

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
    g_signal_connect_swapped(helpMi, "activate", G_CALLBACK(helpMenu), NULL);

    standardView(vbox);

    gtk_widget_show_all (window);
}

GtkApplication* UISetup(char** lines, unsigned long int lncount, size_t grpcnt, grp* dtgrps) {
    loglines.lines = lines;
    loglines.count = lncount;
    datanames.count = grpcnt;
    datanames.names = (char**)malloc(grpcnt*sizeof(char*));
    uidtgrp = dtgrps;
    for (int i = 0; i < grpcnt; i++) {
        datanames.names[i] = (char*)malloc(strlen(dtgrps[i].name)*sizeof(char*));
        strcpy(datanames.names[i], dtgrps[i].name);
    }
    GtkApplication* app = gtk_application_new("org.manifold.logdraw", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
    return app;
}

int UIStart(GtkApplication* app) {
    //int status = g_application_run (G_APPLICATION (app), argc, argv);
    int status = g_application_run (G_APPLICATION (app), 0, NULL);

    g_object_unref (app);
    return status;
}
