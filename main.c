#include <gtk/gtk.h>
#include <string.h>

GtkWidget *window_splash_screen;
GtkWidget *loginWindow;
GtkWidget *accountEntry;
GtkWidget *passwordEntry;
GtkWidget *btnLogin;
GtkWidget *errorLoginLabel;
GtkWidget *windowMain;
GtkWidget *windowMain2;
GtkWidget *lbl_time;
GtkWidget *avatar;
GtkWidget *lbl_name;
GtkWidget *lbl_birthday;
GtkWidget *lbl_level;
GtkWidget *lbl_workplace;
//GtkListStore *listAllPAKN;

typedef struct
{
	char acc[20];
	char pass[20];
	char name[30];
	char birthday[20];
	char role[20];
	char unit[20];
} InfoAccount;

InfoAccount infoAccount;

typedef struct
{
	char nguoiphananh[20];
	char ngayphananh[20];
	char noidung[150];
	int phanloai;
	char trangthai[20];
	char phanhoi[20];
} PAKN;

gboolean timer_handler(gpointer data)
{
	GDateTime *date_time;
	gchar *dt_format;

	date_time = g_date_time_new_now_local();												// get local time
	dt_format = g_date_time_format(date_time, "%H:%M:%S %e/%m/%Y"); // 24hr time format
	gtk_label_set_text(GTK_LABEL(data), dt_format);									// update label
	g_free(dt_format);
	return TRUE;
}

void login()
{
	FILE *fp = fopen("account.txt", "r");
	char read[100], *tmp;
	char name[50] = "Ho ten: \0", birth[50] = "Ngay sinh: \0", role[30] = "Chuc vu: \0", unit[50] = "Don vi: \0";
	int result = 0;
	gchar *acc_entry = gtk_entry_buffer_get_text(gtk_entry_get_buffer(accountEntry));
	gchar *pass_entry = gtk_entry_buffer_get_text(gtk_entry_get_buffer(passwordEntry));

	if (fp == NULL)
	{
		gtk_label_set_text(errorLoginLabel, "Khong the tai du lieu tai khoan");
		return;
	}

	fgets(read, sizeof(read), fp);

	while (!feof(fp))
	{
		if (fgets(read, sizeof(read), fp) == NULL)
			break;

		tmp = strchr(read, '|');
		strncpy(infoAccount.acc, read, tmp - read);
		infoAccount.acc[tmp - read] = '\0';
		strncpy(infoAccount.pass, tmp + 1, strchr(tmp + 1, '|') - tmp - 1);

		if (strcmp(infoAccount.acc, acc_entry) == 0 && strcmp(infoAccount.pass, pass_entry) == 0)
		{
			result = 1;
			tmp = strchr(tmp + 1, '|') + 1;
			sscanf(tmp, "%[^|]|%[^|]|%[^|]|%[^\n]\n", infoAccount.name, infoAccount.birthday, infoAccount.role, infoAccount.unit);
			gtk_window_close(loginWindow);
			strcat(name, infoAccount.name);
			strcat(birth, infoAccount.birthday);
			strcat(role, infoAccount.role);
			strcat(unit, infoAccount.unit);
			gtk_label_set_text(lbl_name, name);
			gtk_label_set_text(lbl_birthday, birth);
			gtk_label_set_text(lbl_level, role);
			gtk_label_set_text(lbl_workplace, unit);
			gtk_widget_show(windowMain);
			return;
		}
	}

	gtk_label_set_text(errorLoginLabel, "Tai khoan hoac mat khau khong dung");

	return;
}

void on_btn_clicked(GtkButton *btn, gpointer data)
{
	//So sánh name widget đang được click với name widget của các widget
	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnLogin") == 0)
	{
		login();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnLogout") == 0)
	{
		gtk_window_close(windowMain);
		gtk_widget_show(loginWindow);
		gtk_text_buffer_set_text(accountEntry, "", 0);
		gtk_text_buffer_set_text(passwordEntry, "", 0);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnAdd") == 0)
	{
		//Code
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnDel") == 0)
	{
		//Code
	}
}

void set_css(void)
{

	GtkCssProvider *css_provider;
	GdkDisplay *display;
	GdkScreen *screen;
	const char *css_file = "style/style.css";
	GError *error = NULL;

	css_provider = gtk_css_provider_new();
	display = gdk_display_get_default();
	screen = gdk_display_get_default_screen(display);
	gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(css_provider),
																						GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	gtk_css_provider_load_from_file(css_provider, g_file_new_for_path(css_file), &error);
	g_object_unref(css_provider);
}

gboolean close_splash_screen(gpointer data)
{
	GtkWidget *progress_bar = data;

	/*Get the current progress*/
	gdouble fraction;
	fraction = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(progress_bar));

	/*Increase the bar by 10% each time this function is called*/
	fraction += 0.4;

	/*Fill in the bar with the new fraction*/
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), fraction);

	if (fraction < 1.0)
		return TRUE;

	gtk_window_close(window_splash_screen);
	gtk_widget_destroy((GtkWidget *)data);
	gtk_widget_destroy(window_splash_screen);
	gtk_widget_show(loginWindow);
	return (FALSE);
}

void gtk_window_destroy()
{
	g_print("See you again!\n");
	gtk_main_quit();
}

int main(int argc,
				 char *argv[])
{
	///////////////////////////////////////////////////////////////////
	GtkBuilder *builder;

	// Khởi tạo GTK
	gtk_init(&argc, &argv);

	// Load giao diện từ file .glade vào biến builder
	builder = gtk_builder_new_from_file("glade/KTPM.glade");
	window_splash_screen = GTK_WINDOW(gtk_builder_get_object(builder, "splash-screen"));
	GtkProgressBar *progressBar = GTK_PROGRESS_BAR(gtk_builder_get_object(builder, "loadingProgress"));
	loginWindow = GTK_WINDOW(gtk_builder_get_object(builder, "loginWindow"));
	accountEntry = GTK_ENTRY(gtk_builder_get_object(builder, "accountEntry"));
	passwordEntry = GTK_ENTRY(gtk_builder_get_object(builder, "passwordEntry"));
	btnLogin = GTK_BUTTON(gtk_builder_get_object(builder, "btnLogin"));
	errorLoginLabel = GTK_LABEL(gtk_builder_get_object(builder, "errorLoginLabel"));
	windowMain = GTK_WINDOW(gtk_builder_get_object(builder, "windowMain"));
	lbl_time = GTK_LABEL(gtk_builder_get_object(builder, "lbl_time"));
	avatar = GTK_IMAGE(gtk_builder_get_object(builder, "avatar"));
	lbl_name = GTK_LABEL(gtk_builder_get_object(builder, "lbl_name"));
	lbl_birthday = GTK_LABEL(gtk_builder_get_object(builder, "lbl_birthday"));
	lbl_level = GTK_LABEL(gtk_builder_get_object(builder, "lbl_level"));
	lbl_workplace = GTK_LABEL(gtk_builder_get_object(builder, "lbl_workplace"));
	// btnLogout = GTK_BUTTON(gtk_builder_get_object(buider, "btnLogout"));
	//listAllPAKN = GTK_LIST_STORE(gtk_builder_get_object(builder, "all_pakn"));

	// Gan time out cho progressBar
	g_timeout_add(500, close_splash_screen, progressBar);
	g_timeout_add_seconds(1, (GSourceFunc)timer_handler, lbl_time);

	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(builder);

	gtk_widget_show(window_splash_screen);

	gtk_main();
}