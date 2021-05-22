#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define INIT_PAKN 100

GtkWidget *window_splash_screen;
GtkWidget *loginWindow;
GtkWidget *accountEntry;
GtkWidget *passwordEntry;
GtkWidget *btnDel;
GtkWidget *btnLogin;
GtkWidget *btnLogout;
GtkWidget *errorLoginLabel;
GtkWidget *windowMain;
GtkWidget *lbl_time;
GtkWidget *avatar;
GtkWidget *lbl_name;
GtkWidget *lbl_birthday;
GtkWidget *lbl_level;
GtkWidget *lbl_workplace;
GtkListStore *listAllPAKN;
GtkListStore *listMoiGhiNhan;
GtkListStore *listChuaGiaiQuyet;
GtkListStore *listDaGiaiQuyet;
GtkWidget *spinner;
GtkWidget *tableQuanLy;
GtkWidget *addWindow;

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
	int id;
	char nguoiphananh[20];
	char ngayphananh[20];
	char noidung[150];
	char phanloai[20];
	int trangthai;
	char phanhoi[20];
} PAKN;

PAKN *pakn;
int sum = 0;

enum
{
	COL_ID = 0,
	COL_NAME,
	COL_DATE,
	COL_TYPE,
	COL_CONTENT,
	COL_STATE,
	COL_RESPONSE,
	NUM_COLS
};

void Display();
void Thongke();

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

gboolean close_spinner(gpointer data)
{
	static int k = 0;
	k++;
	if (k <= 1)
	{
		return TRUE;
	}

	gtk_widget_hide((GtkWindow *)data);
	gtk_window_present(windowMain);
	k = 0;
	return FALSE;
}

// void deletePAKN()
// {
// 	int i, del, j;
// 	// gchar *del_id = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(accountEntry)));
// 	//del = atoi(del_id);
// 	for (i = 0; i < sum; ++i)
// 	{
// 		if (pakn[i].id == del)
// 		{
// 			for (j = i; j < sum - 1; ++j)
// 				pakn[j] = pakn[j + 1];
// 			sum--;
// 			Display();
// 			Thongke();
// 		}
// 	}
// }

void on_row_activated(GtkTreeView *quanly, GtkTreeViewColumn *treeViewColumn, gpointer user_data)
{
	g_print("Double click\n");
}

void deletePAKN()
{
	GtkTreeSelection *select = gtk_tree_view_get_selection(tableQuanLy);

	int countRowSelected = gtk_tree_selection_count_selected_rows(select);

	if (countRowSelected == 0)
	{
	}
}

void InputFilePAKN()
{
	FILE *fp = fopen("pakn.txt", "r");
	if (fp == NULL)
		return;
	char read[300];
	fgets(read, sizeof(read), fp);

	while (!feof(fp))
		if (fgets(read, sizeof(read), fp) != NULL)
		{
			sscanf(read, "%d|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]\n", &pakn[sum].id, pakn[sum].nguoiphananh, pakn[sum].ngayphananh, pakn[sum].phanloai, pakn[sum].noidung, &pakn[sum].trangthai, pakn[sum].phanhoi);
			sum++;
		}
	return;
}

void login()
{
	FILE *fp = fopen("account.txt", "r");
	char read[100], *tmp;
	char name[50] = "Họ và tên: \0", birth[50] = "Ngày sinh: \0", role[30] = "Chức vụ: \0", unit[50] = "Đơn vị: \0";
	int result = 0;
	gchar *acc_entry = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(accountEntry)));
	gchar *pass_entry = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(passwordEntry)));

	if (fp == NULL)
	{
		gtk_label_set_text(errorLoginLabel, "Không thể tải dữ liệu tài khoản");
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
			strcat(name, infoAccount.name);
			strcat(birth, infoAccount.birthday);
			strcat(role, infoAccount.role);
			strcat(unit, infoAccount.unit);
			gtk_label_set_text(lbl_name, name);
			gtk_label_set_text(lbl_birthday, birth);
			gtk_label_set_text(lbl_level, role);
			gtk_label_set_text(lbl_workplace, unit);
			gtk_widget_set_visible(errorLoginLabel, FALSE);
			gtk_widget_hide(loginWindow);
			InputFilePAKN();
			Display();
			Thongke();
			gtk_widget_show(spinner);
			g_timeout_add(500, close_spinner, spinner);
			return;
		}
	}

	gtk_widget_set_visible(errorLoginLabel, TRUE);
	gtk_label_set_text(errorLoginLabel, "Tài khoản hoặc mật khẩu không chính xác");

	return;
}

void Display()
{
	GtkTreeIter iter;
	gtk_list_store_clear(listAllPAKN);
	int i = 0;
	for (i = 0; i < sum; ++i)
	{
		gtk_list_store_append(listAllPAKN, &iter);

		/* Fill fields with some data */

		gtk_list_store_set(listAllPAKN, &iter,
											 COL_ID, pakn[i].id,
											 COL_NAME, pakn[i].nguoiphananh,
											 COL_DATE, pakn[i].ngayphananh,
											 COL_TYPE, pakn[i].phanloai,
											 COL_CONTENT, pakn[i].noidung,
											 COL_STATE, (pakn[i].trangthai == 0) ? "Moi ghi nhan" : (pakn[i].trangthai == 1) ? "Chua giai quyet"
																																																			 : "Da giai quyet",

											 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
											 -1);
	}
}

void Thongke()
{
	int i;
	GtkTreeIter iter1, iter2, iter3;

	for (i = 0; i < sum; ++i)
	{
		switch (pakn[i].trangthai)
		{
		case 0:
			gtk_list_store_append(listMoiGhiNhan, &iter1);

			/* Fill fields with some data */

			gtk_list_store_set(listMoiGhiNhan, &iter1,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_CONTENT, pakn[i].noidung,
												 -1);
			break;

		case 1:
			gtk_list_store_append(listChuaGiaiQuyet, &iter2);

			/* Fill fields with some data */

			gtk_list_store_set(listChuaGiaiQuyet, &iter2,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_CONTENT, pakn[i].noidung,
												 -1);
			break;

		default:
			gtk_list_store_append(listDaGiaiQuyet, &iter3);

			/* Fill fields with some data */

			gtk_list_store_set(listDaGiaiQuyet, &iter3,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_CONTENT, pakn[i].noidung,
												 COL_RESPONSE - 1, pakn[i].phanhoi,
												 -1);
		}
	}
}

void logout()
{
	gtk_widget_hide(windowMain);
	gtk_entry_set_text(GTK_ENTRY(accountEntry), "");
	gtk_entry_set_text(GTK_ENTRY(passwordEntry), "");
	gtk_list_store_clear(listAllPAKN);
	gtk_list_store_clear(listMoiGhiNhan);
	gtk_list_store_clear(listChuaGiaiQuyet);
	gtk_list_store_clear(listDaGiaiQuyet);
	gtk_widget_show(loginWindow);
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
		logout();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnAdd") == 0)
	{
		gtk_window_present(addWindow);
		AddPAKN();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnDel") == 0)
	{
		deletePAKN();
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
	btnDel = GTK_BUTTON(gtk_builder_get_object(builder, "btnDel"));
	errorLoginLabel = GTK_LABEL(gtk_builder_get_object(builder, "errorLoginLabel"));
	windowMain = GTK_WINDOW(gtk_builder_get_object(builder, "windowMain"));
	lbl_time = GTK_LABEL(gtk_builder_get_object(builder, "lbl_time"));
	avatar = GTK_IMAGE(gtk_builder_get_object(builder, "avatar"));
	lbl_name = GTK_LABEL(gtk_builder_get_object(builder, "lbl_name"));
	lbl_birthday = GTK_LABEL(gtk_builder_get_object(builder, "lbl_birthday"));
	lbl_level = GTK_LABEL(gtk_builder_get_object(builder, "lbl_level"));
	lbl_workplace = GTK_LABEL(gtk_builder_get_object(builder, "lbl_workplace"));
	btnLogout = GTK_BUTTON(gtk_builder_get_object(builder, "btnLogout"));
	listAllPAKN = GTK_LIST_STORE(gtk_builder_get_object(builder, "all_pakn"));
	listMoiGhiNhan = GTK_LIST_STORE(gtk_builder_get_object(builder, "moighinhan"));
	listChuaGiaiQuyet = GTK_LIST_STORE(gtk_builder_get_object(builder, "chuagiaiquyet"));
	listDaGiaiQuyet = GTK_LIST_STORE(gtk_builder_get_object(builder, "dagiaiquyet"));
	tableQuanLy = GTK_TREE_VIEW(gtk_builder_get_object(builder, "quanly"));
	spinner = GTK_WINDOW(gtk_builder_get_object(builder, "spinner"));
	addWindow = GTK_WINDOW(gtk_builder_get_object(builder, "addWindow"));

	pakn = malloc(INIT_PAKN * sizeof(PAKN));

	// Gan time out cho progressBar
	g_timeout_add(500, close_splash_screen, progressBar);
	g_timeout_add_seconds(1, (GSourceFunc)timer_handler, lbl_time);

	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(builder);

	gtk_widget_show(window_splash_screen);

	gtk_main();
}