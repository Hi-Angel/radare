/* EntryDialog.c generated by valac, the Vala compiler
 * generated from EntryDialog.vala, do not modify */


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>


#define TYPE_ENTRY_DIALOG (entry_dialog_get_type ())
#define ENTRY_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_ENTRY_DIALOG, EntryDialog))
#define ENTRY_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_ENTRY_DIALOG, EntryDialogClass))
#define IS_ENTRY_DIALOG(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_ENTRY_DIALOG))
#define IS_ENTRY_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_ENTRY_DIALOG))
#define ENTRY_DIALOG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_ENTRY_DIALOG, EntryDialogClass))

typedef struct _EntryDialog EntryDialog;
typedef struct _EntryDialogClass EntryDialogClass;
typedef struct _EntryDialogPrivate EntryDialogPrivate;
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))

struct _EntryDialog {
	GtkDialog parent_instance;
	EntryDialogPrivate * priv;
};

struct _EntryDialogClass {
	GtkDialogClass parent_class;
};

struct _EntryDialogPrivate {
	GtkLabel* label;
	GtkEntry* entry;
};


static gpointer entry_dialog_parent_class = NULL;

GType entry_dialog_get_type (void);
#define ENTRY_DIALOG_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_ENTRY_DIALOG, EntryDialogPrivate))
enum  {
	ENTRY_DIALOG_DUMMY_PROPERTY
};
EntryDialog* entry_dialog_new (void);
EntryDialog* entry_dialog_construct (GType object_type);
void entry_dialog_set_label (EntryDialog* self, const char* text);
static void entry_dialog_ok_clicked (EntryDialog* self);
static void entry_dialog_on_response (EntryDialog* self, GtkDialog* source, gint response_id);
static gint entry_dialog_main (char** args, int args_length1);
static void _entry_dialog_on_response_gtk_dialog_response (EntryDialog* _sender, gint response_id, gpointer self);
static void _gtk_main_quit_gtk_object_destroy (EntryDialog* _sender, gpointer self);
static GObject * entry_dialog_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties);
static void entry_dialog_finalize (GObject* obj);



EntryDialog* entry_dialog_construct (GType object_type) {
	EntryDialog * self;
	self = g_object_newv (object_type, 0, NULL);
	gtk_dialog_add_button ((GtkDialog*) self, GTK_STOCK_CANCEL, (gint) GTK_RESPONSE_CANCEL);
	gtk_dialog_add_button ((GtkDialog*) self, GTK_STOCK_OK, (gint) GTK_RESPONSE_OK);
	return self;
}


EntryDialog* entry_dialog_new (void) {
	return entry_dialog_construct (TYPE_ENTRY_DIALOG);
}


void entry_dialog_set_label (EntryDialog* self, const char* text) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (text != NULL);
	gtk_label_set_text (self->priv->label, text);
}


static void entry_dialog_on_response (EntryDialog* self, GtkDialog* source, gint response_id) {
	g_return_if_fail (self != NULL);
	g_return_if_fail (source != NULL);
	switch (response_id) {
		case GTK_RESPONSE_CANCEL:
		{
			gtk_object_destroy ((GtkObject*) self);
			break;
		}
		case GTK_RESPONSE_OK:
		{
			entry_dialog_ok_clicked (self);
			gtk_object_destroy ((GtkObject*) self);
			break;
		}
	}
}


static void entry_dialog_ok_clicked (EntryDialog* self) {
	g_return_if_fail (self != NULL);
	g_print ("VARIABLE=%s\n", gtk_entry_get_text (self->priv->entry));
}


static gint entry_dialog_main (char** args, int args_length1) {
	gint result;
	EntryDialog* dialog;
	gtk_init (&args_length1, &args);
	dialog = g_object_ref_sink (entry_dialog_new ());
	if (args_length1 > 1) {
		gtk_window_set_title ((GtkWindow*) dialog, args[1]);
	}
	if (args_length1 > 2) {
		entry_dialog_set_label (dialog, args[2]);
	}
	gtk_widget_show_all ((GtkWidget*) dialog);
	gtk_main ();
	result = 0;
	_g_object_unref0 (dialog);
	return result;
}


int main (int argc, char ** argv) {
	g_type_init ();
	return entry_dialog_main (argv, argc);
}


static void _entry_dialog_on_response_gtk_dialog_response (EntryDialog* _sender, gint response_id, gpointer self) {
	entry_dialog_on_response (self, _sender, response_id);
}


static void _gtk_main_quit_gtk_object_destroy (EntryDialog* _sender, gpointer self) {
	gtk_main_quit ();
}


static GObject * entry_dialog_constructor (GType type, guint n_construct_properties, GObjectConstructParam * construct_properties) {
	GObject * obj;
	GObjectClass * parent_class;
	EntryDialog * self;
	parent_class = G_OBJECT_CLASS (entry_dialog_parent_class);
	obj = parent_class->constructor (type, n_construct_properties, construct_properties);
	self = ENTRY_DIALOG (obj);
	{
		GtkHBox* _hbox0;
		GtkHBox* _tmp0_;
		GtkLabel* _tmp1_;
		GtkEntry* _tmp2_;
		GtkVBox* _tmp3_;
		GtkVBox* _tmp4_;
		_hbox0 = NULL;
		_hbox0 = (_tmp0_ = g_object_ref_sink ((GtkHBox*) gtk_hbox_new (FALSE, 10)), _g_object_unref0 (_hbox0), _tmp0_);
		self->priv->label = (_tmp1_ = g_object_ref_sink ((GtkLabel*) gtk_label_new_with_mnemonic ("Input:")), _g_object_unref0 (self->priv->label), _tmp1_);
		self->priv->entry = (_tmp2_ = g_object_ref_sink ((GtkEntry*) gtk_entry_new ()), _g_object_unref0 (self->priv->entry), _tmp2_);
		gtk_window_set_title ((GtkWindow*) self, "gradare dialog");
		gtk_dialog_set_has_separator ((GtkDialog*) self, FALSE);
		gtk_container_set_border_width ((GtkContainer*) self, (guint) 5);
		g_object_set ((GtkWindow*) self, "default-width", 300, NULL);
		g_object_set ((GtkWindow*) self, "default-height", 80, NULL);
		g_signal_connect_object ((GtkDialog*) self, "response", (GCallback) _entry_dialog_on_response_gtk_dialog_response, self, 0);
		g_signal_connect ((GtkObject*) self, "destroy", (GCallback) _gtk_main_quit_gtk_object_destroy, NULL);
		gtk_label_set_mnemonic_widget (self->priv->label, (GtkWidget*) self->priv->entry);
		gtk_box_pack_start ((GtkBox*) _hbox0, (GtkWidget*) self->priv->label, FALSE, TRUE, (guint) 0);
		gtk_container_add ((GtkContainer*) _hbox0, (GtkWidget*) self->priv->entry);
		gtk_box_pack_start ((GtkBox*) (_tmp3_ = ((GtkDialog*) self)->vbox, GTK_IS_VBOX (_tmp3_) ? ((GtkVBox*) _tmp3_) : NULL), (GtkWidget*) _hbox0, FALSE, TRUE, (guint) 0);
		gtk_box_set_spacing ((GtkBox*) (_tmp4_ = ((GtkDialog*) self)->vbox, GTK_IS_VBOX (_tmp4_) ? ((GtkVBox*) _tmp4_) : NULL), 10);
		_g_object_unref0 (_hbox0);
	}
	return obj;
}


static void entry_dialog_class_init (EntryDialogClass * klass) {
	entry_dialog_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (EntryDialogPrivate));
	G_OBJECT_CLASS (klass)->constructor = entry_dialog_constructor;
	G_OBJECT_CLASS (klass)->finalize = entry_dialog_finalize;
}


static void entry_dialog_instance_init (EntryDialog * self) {
	self->priv = ENTRY_DIALOG_GET_PRIVATE (self);
}


static void entry_dialog_finalize (GObject* obj) {
	EntryDialog * self;
	self = ENTRY_DIALOG (obj);
	_g_object_unref0 (self->priv->label);
	_g_object_unref0 (self->priv->entry);
	G_OBJECT_CLASS (entry_dialog_parent_class)->finalize (obj);
}


GType entry_dialog_get_type (void) {
	static GType entry_dialog_type_id = 0;
	if (entry_dialog_type_id == 0) {
		static const GTypeInfo g_define_type_info = { sizeof (EntryDialogClass), (GBaseInitFunc) NULL, (GBaseFinalizeFunc) NULL, (GClassInitFunc) entry_dialog_class_init, (GClassFinalizeFunc) NULL, NULL, sizeof (EntryDialog), 0, (GInstanceInitFunc) entry_dialog_instance_init, NULL };
		entry_dialog_type_id = g_type_register_static (GTK_TYPE_DIALOG, "EntryDialog", &g_define_type_info, 0);
	}
	return entry_dialog_type_id;
}




