#ifndef GTKUI_H
#define GTKUI_H
#include <gtk/gtk.h>
#include <regex.h>
#include "datapoint.h"


GtkApplication* UISetup(char** lines, unsigned long int lncount, size_t grpcnt, grp** dtgrps);
int UIStart(GtkApplication* app);
void logScrollSetup(GtkCssProvider* css);
void logScrollInsert(GtkCssProvider* css, GtkWidget* loglist, char* line);
void logScrollCleanup(GtkCssProvider* css);

#endif
