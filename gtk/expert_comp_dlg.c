/* expert_comp_dlg.c
 * expert_comp_dlg   2005 Greg Morris
 *
 * $Id$
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <string.h>

#include <gtk/gtk.h>

#include <epan/packet_info.h>
#include <epan/tap.h>
#include <epan/stat_cmd_args.h>

#include "../register.h"
#include "../simple_dialog.h"
#include "../globals.h"
#include "../stat_menu.h"

#include "gtk/gui_utils.h"
#include "gtk/dlg_utils.h"
#include "gtk/expert_comp_table.h"
#include "gtk/gui_stat_menu.h"
#include "gtk/help_dlg.h"
#include "gtk/expert_comp_dlg.h"
#include "gtk/stock_icons.h"
#include "gtk/main.h"


/* used to keep track of the statistics for an entire program interface */
typedef struct _expert_comp_dlg_t {
    GtkWidget *win;
    GtkWidget *chat_label;
    GtkWidget *note_label;
    GtkWidget *warn_label;
    GtkWidget *error_label;
    GtkWidget *all_label;
    error_equiv_table chat_table;
    error_equiv_table note_table;
    error_equiv_table warn_table;
    error_equiv_table error_table;
    guint32 disp_events;
    guint32 chat_events;
    guint32 note_events;
    guint32 warn_events;
    guint32 error_events;
} expert_comp_dlg_t;

static GtkWidget  *expert_comp_dlg_w = NULL;

static void
error_set_title(expert_comp_dlg_t *ss)
{
    char *title;

    title = g_strdup_printf("Expert Info Composite: %s",
        cf_get_display_name(&cfile));
    gtk_window_set_title(GTK_WINDOW(ss->win), title);
    g_free(title);
}

static void
error_reset(void *pss)
{
    expert_comp_dlg_t *ss=(expert_comp_dlg_t *)pss;
    gchar *buf;

    ss->error_events = 0;
    ss->warn_events = 0;
    ss->note_events = 0;
    ss->chat_events = 0;
    ss->disp_events = 0;

    reset_error_table_data(&ss->error_table);
    buf = g_strdup_printf("Errors: %u (0)", ss->error_table.num_procs);
    gtk_label_set_text( GTK_LABEL(ss->error_label), buf);
    g_free(buf);

    reset_error_table_data(&ss->warn_table);
    buf = g_strdup_printf("Warnings: %u (0)", ss->warn_table.num_procs);
    gtk_label_set_text( GTK_LABEL(ss->warn_label), buf);
    g_free(buf);

    reset_error_table_data(&ss->note_table);
    buf = g_strdup_printf("Notes: %u (0)", ss->note_table.num_procs);
    gtk_label_set_text( GTK_LABEL(ss->note_label), buf);
    g_free(buf);

    reset_error_table_data(&ss->chat_table);
    buf = g_strdup_printf("Chats: %u (0)", ss->chat_table.num_procs);
    gtk_label_set_text( GTK_LABEL(ss->chat_label), buf);
    g_free(buf);

    gtk_label_set_text( GTK_LABEL(ss->all_label), "Details: 0");
    error_set_title(ss);
}

static int
error_packet(void *pss, packet_info *pinfo _U_, epan_dissect_t *edt _U_, const void *prv)
{
    expert_comp_dlg_t *ss=(expert_comp_dlg_t *)pss;
    const expert_info_t *error_pkt=prv;

    /* if return value is 0 then no error */
    if(error_pkt==NULL){
        return 0;
    }

    switch (error_pkt->severity) {
    case PI_ERROR:
        ss->disp_events++;
        ss->error_events++;
        init_error_table_row(&ss->error_table, error_pkt);
        break;
    case PI_WARN:
        ss->disp_events++;
        ss->warn_events++;
        init_error_table_row(&ss->warn_table, error_pkt);
        break;
    case PI_NOTE:
        ss->disp_events++;
        ss->note_events++;
        init_error_table_row(&ss->note_table, error_pkt);
        break;
    case PI_CHAT:
        ss->disp_events++;
        ss->chat_events++;
        init_error_table_row(&ss->chat_table, error_pkt);
        break;
    default:
        return 0; /* Don't draw */
    }
    return 1; /* Draw */
}

static void
expert_comp_draw(void *data)
{
    gchar *buf = NULL;

    expert_comp_dlg_t *ss=(expert_comp_dlg_t *)data;

    buf = g_strdup_printf("Errors: %u (%u)", ss->error_table.num_procs, ss->error_events);
    gtk_label_set_text( GTK_LABEL(ss->error_label), buf);
    g_free(buf);

    buf = g_strdup_printf("Warnings: %u (%u)", ss->warn_table.num_procs, ss->warn_events);
    gtk_label_set_text( GTK_LABEL(ss->warn_label), buf);
    g_free(buf);

    buf = g_strdup_printf("Notes: %u (%u)", ss->note_table.num_procs, ss->note_events);
    gtk_label_set_text( GTK_LABEL(ss->note_label), buf);
    g_free(buf);

    buf = g_strdup_printf("Chats: %u (%u)", ss->chat_table.num_procs, ss->chat_events);
    gtk_label_set_text( GTK_LABEL(ss->chat_label), buf);
    g_free(buf);

    buf = g_strdup_printf("Details: %u", ss->disp_events);
    gtk_label_set_text( GTK_LABEL(ss->all_label), buf);
    g_free(buf);
    
}

static void
win_destroy_cb(GtkWindow *win _U_, gpointer data)
{
    expert_comp_dlg_t *ss=(expert_comp_dlg_t *)data;

    protect_thread_critical_region();
    remove_tap_listener(ss);
    unprotect_thread_critical_region();

    if (expert_comp_dlg_w != NULL) {
        window_destroy(expert_comp_dlg_w);
        expert_comp_dlg_w = NULL;
    }

    free_error_table_data(&ss->error_table);
    free_error_table_data(&ss->warn_table);
    free_error_table_data(&ss->note_table);
    free_error_table_data(&ss->chat_table);
    g_free(ss);

}

static void
expert_comp_init(const char *optarg _U_, void* userdata _U_)
{
    expert_comp_dlg_t *ss;
    const char *filter=NULL;
    GString *error_string;
    GtkWidget *temp_page;
    GtkWidget *main_nb;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *bbox;
    GtkWidget *close_bt;
    GtkWidget *help_bt;
    expert_tapdata_t *etd;
    GtkTooltips *tooltips = gtk_tooltips_new();

    ss=g_malloc(sizeof(expert_comp_dlg_t));

    ss->disp_events = 0;
    ss->chat_events = 0;
    ss->note_events = 0;
    ss->warn_events = 0;
    ss->error_events = 0;

	expert_comp_dlg_w = ss->win=dlg_window_new("err");  /* transient_for top_level */
	gtk_window_set_destroy_with_parent (GTK_WINDOW(ss->win), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(ss->win), 700, 300);

    error_set_title(ss);

    vbox=gtk_vbox_new(FALSE, 3);
    gtk_container_add(GTK_CONTAINER(ss->win), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

    main_nb = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), main_nb, TRUE, TRUE, 0);

    /* We must display TOP LEVEL Widget before calling init_table() */
    gtk_widget_show_all(ss->win);

    /* Errors */
    temp_page = gtk_vbox_new(FALSE, 6);
    ss->error_label = gtk_label_new("Errors: 0/y");
    gtk_widget_show(ss->error_label);
    hbox = gtk_hbox_new(FALSE, 3);
    gtk_container_add(GTK_CONTAINER(hbox), ss->error_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(main_nb), temp_page, hbox);
    init_error_table(&ss->error_table, 0, temp_page);

    /* Warnings */
    temp_page = gtk_vbox_new(FALSE, 6);
    ss->warn_label = gtk_label_new("Warnings: 0/y");
    gtk_widget_show(ss->warn_label);
    hbox = gtk_hbox_new(FALSE, 3);
    gtk_container_add(GTK_CONTAINER(hbox), ss->warn_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(main_nb), temp_page, hbox);
    init_error_table(&ss->warn_table, 0, temp_page);

    /* Notes */
    temp_page = gtk_vbox_new(FALSE, 6);
    ss->note_label = gtk_label_new("Notes: 0/y");
    gtk_widget_show(ss->note_label);
    hbox = gtk_hbox_new(FALSE, 3);
    gtk_container_add(GTK_CONTAINER(hbox), ss->note_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(main_nb), temp_page, hbox);
    init_error_table(&ss->note_table, 0, temp_page);

    /* Chat */
    temp_page = gtk_vbox_new(FALSE, 6);
    ss->chat_label = gtk_label_new("Chats: 0/y");
    gtk_widget_show(ss->chat_label);
    hbox = gtk_hbox_new(FALSE, 3);
    gtk_container_add(GTK_CONTAINER(hbox), ss->chat_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(main_nb), temp_page, hbox);
    init_error_table(&ss->chat_table, 0, temp_page);

    /* Details */
    temp_page = gtk_vbox_new(FALSE, 6);
    ss->all_label = gtk_label_new("Details: 0");
    gtk_notebook_append_page(GTK_NOTEBOOK(main_nb), temp_page, ss->all_label);

    etd = expert_dlg_new_table();
    etd->label=gtk_label_new("Please wait ...");
    gtk_misc_set_alignment(GTK_MISC(etd->label), 0.0f, 0.5f);

    etd->win=ss->win;
    expert_dlg_init_table(etd, temp_page);

    /* Add tap listener functions for expert details, From expert_dlg.c*/
    error_string=register_tap_listener("expert", etd, NULL /* fstring */,
                                       TL_REQUIRES_PROTO_TREE,
                                       expert_dlg_reset,
                                       expert_dlg_packet,
                                       expert_dlg_draw);
    if(error_string){
        simple_dialog(ESD_TYPE_ERROR, ESD_BTN_OK, "%s", error_string->str);
        g_string_free(error_string, TRUE);
        g_free(etd);
        return;
    }

    g_signal_connect(etd->win, "delete_event", G_CALLBACK(window_delete_event_cb), NULL);
    g_signal_connect(etd->win, "destroy", G_CALLBACK(expert_dlg_destroy_cb), etd);

    /* Register the tap listener */

    error_string=register_tap_listener("expert", ss, filter, 0, error_reset, error_packet, expert_comp_draw);
    if(error_string){
        simple_dialog(ESD_TYPE_ERROR, ESD_BTN_OK, "%s", error_string->str);
        g_string_free(error_string, TRUE);
        g_free(ss);
        return;
    }

    /* Button row. */
    bbox = dlg_button_row_new(GTK_STOCK_CLOSE, GTK_STOCK_HELP, NULL);
    gtk_box_pack_end(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

    close_bt = g_object_get_data(G_OBJECT(bbox), GTK_STOCK_CLOSE);
    window_set_cancel_button(ss->win, close_bt, window_cancel_button_cb);

    help_bt = g_object_get_data(G_OBJECT(bbox), GTK_STOCK_HELP);
    g_signal_connect(help_bt, "clicked", G_CALLBACK(topic_cb), (gpointer)HELP_EXPERT_INFO_DIALOG);
    gtk_tooltips_set_tip (tooltips, help_bt, "Show topic specific help", NULL);

    g_signal_connect(ss->win, "delete_event", G_CALLBACK(window_delete_event_cb), NULL);
    g_signal_connect(ss->win, "destroy", G_CALLBACK(win_destroy_cb), ss);

    gtk_widget_show_all(ss->win);
    window_present(ss->win);

    /*
     * At least at present, the only information the tap listener appears
     * to care about is available regardless of whether the protocol tree
     * is being built, so we don't appear to need to have the protocol
     * tree built.
     *
     * This means we can use cf_retap_packets(), even though it will only
     * build the protocol tree if at least one tap has a filter in place.
     * cf_retap_packets() is faster than cf_redissect_packets(), as it
     * assumes we didn't change anything that would cause any packets to
     * dissect differently, and thus doesn't redo the packet display.
     */
    cf_retap_packets(&cfile);

    /* This will bring up the progress bar
     * Put our window back in front
     */
    gdk_window_raise(ss->win->window);
	/* Set the lable text */
	expert_comp_draw(ss);
}

void
expert_comp_dlg_cb(GtkWidget *w _U_, gpointer d _U_)
{
    if (expert_comp_dlg_w) {
          reactivate_window(expert_comp_dlg_w);
    } else {
          expert_comp_init("", NULL);
    }
}

void
register_tap_listener_expert_comp(void)
{
    register_stat_cmd_arg("expert_comp", expert_comp_init,NULL);
    register_stat_menu_item_stock("Expert Info _Composite",
        REGISTER_ANALYZE_GROUP_UNSORTED, WIRESHARK_STOCK_EXPERT_INFO,
        expert_comp_dlg_cb, NULL, NULL, NULL);
}
