#include <netcdf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "../include/callbacks.h"
#include "../include/netcdf_api.h"
#include "../include/sdl_api.h"


extern GtkBuilder *builder;
bool is_file_selected = false;
bool filter = true;
bool image_mode = true;


G_MODULE_EXPORT void on_window_ceilometer_destroy(void)
{
	gtk_main_quit();
}


G_MODULE_EXPORT void on_file_netcdf_file_set(void)
{
	is_file_selected = true;
}


G_MODULE_EXPORT void on_check_filter_toggled(void)
{
	if (filter)
		filter = false;
	else
		filter = true;
}


G_MODULE_EXPORT void on_radio_image_toggled(void)
{
	image_mode = true;
}


G_MODULE_EXPORT void on_radio_measure_toggled(void)
{
	image_mode = false;
}


G_MODULE_EXPORT void on_button_validation_clicked(void)
{
	// Récupération du nom du fichier
	const char *filename = NULL;
	if (is_file_selected)
	{
		GtkFileChooser *file_chooser = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "file_netcdf"));
		filename = gtk_file_chooser_get_filename(file_chooser);
	}
	else return;

	// Récupération de la variable
	GtkComboBox *combo = GTK_COMBO_BOX(gtk_builder_get_object(builder, "combo_ceilometer_var"));
	const char *var = NULL;
	switch(gtk_combo_box_get_active(combo))
	{
		case 0:
			var = "linear_depol_ratio";
			break;

		case 1:
			var = "beta_att";
			break;

		case 2:
			var = "p_pol";
			break;

		case 3:
			var = "x_pol";
			break;
	}

	size_t NTIME, NRANGE;
	int year, month, day;
	netcdf_get_metadata(filename, &NTIME, &NRANGE, &year, &month, &day);

	char date[11];
	sprintf(date, "%d-%d-%d", day, month, year);

	GtkLabel *label_status = GTK_LABEL(gtk_builder_get_object(builder, "label_status"));
	gtk_label_set_text(label_status, "en cours de traitement");
	gtk_main_iteration();
	sleep(0.1);
	gtk_main_iteration();

	// Récupération des données
	float *data = malloc(NTIME * NRANGE * sizeof(float));
	float *alt = malloc(NRANGE * sizeof(float));
	netcdf_get_data(data, alt, filename, var);

	// Récupération du mode d'ouverture et traitement des données
	float minimum = 0, maximum = 0;
	if (filter)
	{
		GtkSpinButton *spin_minimum = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_minimum"));
		minimum = (float) gtk_spin_button_get_value(spin_minimum);

		GtkSpinButton *spin_maximum = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spin_maximum"));
		maximum = (float) gtk_spin_button_get_value(spin_maximum);

		if (maximum <= minimum) return;
	}

	if (image_mode)
		sdl_image(data, alt, minimum, maximum, filter, var, date, NTIME, NRANGE);
	else
		sdl_measure(data, alt, minimum, maximum, filter, var, date, NTIME, NRANGE);

	free(data);
	free(alt);

	gtk_label_set_text(label_status, "en attente de validation");
	gtk_main_iteration();
	sleep(0.1);
	gtk_main_iteration();

}