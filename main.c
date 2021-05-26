#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "smtp.h"

#define INIT_PAKN 100

#define MAIL_SERVER "smtp.gmail.com"
#define MAIL_PORT "587"
#define MAIL_CONNECTION_SECURITY SMTP_SECURITY_STARTTLS
#define MAIL_FLAGS (SMTP_DEBUG | \
										SMTP_NO_CERT_VERIFY) /* Do not verify cert. */
#define MAIL_CAFILE NULL
#define MAIL_AUTH SMTP_AUTH_PLAIN
#define MAIL_USER "tuyenldjp@gmail.com"
#define MAIL_PASS ""
#define MAIL_FROM "noreply@example.org"

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
GtkWidget *entryAddName;
GtkWidget *entryAddType;
GtkWidget *addContent;
GtkWidget *revealer;
GtkWidget *revealer1;
GtkWidget *revealer2;
GtkWidget *lblNotifi;
GtkWidget *lblNotifiAdd;
GtkWidget *lblNotifiEditView;
GtkWidget *searchEntry;
GtkWidget *btnCombine;
GtkWidget *btnSendSuperior;
GtkWidget *quyComboText;
GtkWidget *namComboText;
GtkWidget *btnFilter;
GtkWidget *btnPrintAnalysis;
GtkWidget *quanly;
GtkWidget *stackAnalysis;
GtkWidget *editAndViewWindow;
GtkWidget *entryEditViewName;
GtkWidget *entryEditViewType;
GtkWidget *textviewEditViewContent;
GtkWidget *lblEditViewResponse;
GtkWidget *textviewEditViewResponse;
GtkWidget *btnSave;
GtkWidget *btnCloseEditViewWindow;
GtkWidget *scrollForResponse;
GtkWidget *entryEmailEditView;
GtkWidget *entryAddEmail;
GtkWidget *entryNumPAKN;
GtkWidget *btnSendResponse;
GtkWidget *btnLoadFile;
GtkWidget *btnCloseLoadFile;
GtkWidget *dialogFileChooser;
GtkTreeSortable *sortableQuanLy;

struct smtp *smtp;

int isSMTPSended = 0;
int sttInPAKNArr;

gboolean inSession = FALSE;

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
	long id;
	char nguoiphananh[500];
	char ngayphananh[500];
	char noidung[250];
	char phanloai[60];
	int trangthai;
	char phanhoi[200];
	char email[200];
	int solan;
} PAKN;

PAKN *pakn;
int sum = 0;

enum
{
	COL_ID = 0,
	COL_NAME,
	COL_EMAIL,
	COL_DATE,
	COL_TYPE,
	COL_NUM,
	COL_CONTENT,
	COL_STATE,
	COL_RESPONSE,
	NUM_COLS
};

void Display();
void Thongke();
void showNotification(GtkRevealer *, GtkLabel *, gchar *, gint);
void closeNotification(GtkRevealer *revealer);
long CreateID();
void Display();
void Search();
void Send();
void on_searchEntry_delete_text(GtkEditable *searchEntry, gint, gpointer user_data);
void DeletePAKN(int i);
void ExportToFile();

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

gboolean close_revealer(gpointer data)
{
	static int r = 0;
	r++;
	if (r <= 1)
	{
		return TRUE;
	}

	r = 0;
	closeNotification(data);
	return FALSE;
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
	inSession = TRUE;
	gtk_window_present(windowMain);
	k = 0;
	showNotification(revealer, lblNotifi, "Chào mừng bạn đã quay lại!", 1000);
	return FALSE;
}

gboolean on_searchEntry_key_press_event(GtkWidget *searchEntry, GdkEventKey *key, gpointer user_data)
{
	if (key)
	{
		//Kiểm tra xem nút nhấn có phải là nút Backspace hay Delete hay không
		if (key->keyval == GDK_KEY_BackSpace || key->keyval == GDK_KEY_Delete)
		{
			// Khi xóa từ sẽ đặt rỗng cho meaningViewBuff
			Display();
			Thongke();
		}
	}

	//Phải trả về là false thì searchEntry mới có thể hiện chữ được. Còn trả về là TRUE thì không hiện chữ được
	return FALSE;
}

int getPriority(char *name)
{
	int ret = -1;
	if (g_utf8_collate(name, "Mới ghi nhận") == 0)
		ret = 0;
	if (g_utf8_collate(name, "Chưa giải quyết") == 0)
		ret = 1;
	if (g_utf8_collate(name, "Đã giải quyết") == 0)
		ret = 2;
	return ret;
}

gint sort_iter_compare_func(GtkTreeModel *model,
														GtkTreeIter *a,
														GtkTreeIter *b,
														gpointer userdata)
{
	gint sortcol = GPOINTER_TO_INT(userdata);
	gint ret = 0;
	int i1, i2;

	switch (sortcol)
	{
	case COL_STATE:
	{
		gchar *name1, *name2;

		gtk_tree_model_get(model, a, COL_STATE, &name1, -1);
		gtk_tree_model_get(model, b, COL_STATE, &name2, -1);

		if (name1 == NULL || name2 == NULL)
		{
			if (name1 == NULL && name2 == NULL)
				break; /* both equal => ret = 0 */

			ret = (name1 == NULL) ? -1 : 1;
		}
		else
		{
			ret = getPriority(name1) - getPriority(name2);
		}

		g_free(name1);
		g_free(name2);
	}
	break;
	default:
		g_return_val_if_reached(0);
	}

	return ret;
}

int connectSMTP()
{
	int rc;
	rc = smtp_open(MAIL_SERVER,
								 MAIL_PORT,
								 MAIL_CONNECTION_SECURITY,
								 MAIL_FLAGS,
								 MAIL_CAFILE,
								 &smtp);
	rc = smtp_auth(smtp,
								 MAIL_AUTH,
								 MAIL_USER,
								 MAIL_PASS);
	return rc;
}

int closeSMTP()
{
	int rc;
	rc = smtp_close(smtp);
	return rc;
}

void clearAllSMTP()
{
	smtp_header_clear_all(smtp);
	smtp_address_clear_all(smtp);
	smtp_attachment_clear_all(smtp);
}

int sendMail(char toEmail[], char toName[], char subject[], char body[], char attachment[])
{
	int rc;

	char fromName[100];

	sprintf(fromName, "%s - %s - %s", infoAccount.name, infoAccount.role, infoAccount.unit);

	rc = smtp_address_add(smtp,
												SMTP_ADDRESS_FROM,
												MAIL_FROM,
												fromName);
	rc = smtp_address_add(smtp,
												SMTP_ADDRESS_TO,
												toEmail,
												toName);
	rc = smtp_header_add(smtp,
											 "Subject",
											 subject);
	if (strlen(attachment) != 0)
	{
		FILE *f = fopen(attachment, "r");
		if (f == NULL)
		{
			showNotification(revealer, lblNotifi, "Lỗi!!!", 1000);
			return;
		}
		rc = smtp_attachment_add_fp(smtp, "Attachment.txt", f);
	}
	rc = smtp_mail(smtp,
								 body);

	return 0;
}

void showNotification(GtkRevealer *revealer, GtkLabel *lblNotification, gchar *str, gint timeout)
{
	gtk_label_set_text(lblNotification, str);
	g_timeout_add(timeout, close_revealer, revealer);
	gtk_revealer_set_reveal_child(revealer, TRUE);
}

void closeNotification(GtkRevealer *revealer)
{
	gtk_revealer_set_reveal_child(revealer, FALSE);
}

void Popup(long id, gboolean canEdit)
{
	char solan[5];
	for (int i = sum - 1; i >= 0; --i)
	{
		if (pakn[i].id == id)
		{
			canEdit ? gtk_window_set_title(editAndViewWindow, "Edit Window") : gtk_window_set_title(editAndViewWindow, "View Window");
			gtk_widget_set_visible(lblEditViewResponse, !canEdit);
			gtk_widget_set_visible(scrollForResponse, !canEdit);
			gtk_widget_set_visible(btnSave, canEdit);
			gtk_editable_set_editable(GTK_EDITABLE(entryEditViewName), canEdit);
			gtk_editable_set_editable(GTK_EDITABLE(entryEditViewType), canEdit);
			gtk_editable_set_editable(GTK_EDITABLE(entryEmailEditView), canEdit);
			gtk_text_view_set_editable(textviewEditViewContent, canEdit);
			gtk_text_view_set_editable(textviewEditViewResponse, canEdit);
			gtk_entry_set_text(entryEditViewName, pakn[i].nguoiphananh);
			gtk_entry_set_text(entryEditViewType, pakn[i].phanloai);
			gtk_entry_set_text(entryEmailEditView, pakn[i].email);
			sprintf(solan, "%d", pakn[i].solan);
			gtk_entry_set_text(entryNumPAKN, solan);
			gtk_text_buffer_set_text(gtk_text_view_get_buffer(textviewEditViewContent), pakn[i].noidung, -1);
			gtk_text_buffer_set_text(gtk_text_view_get_buffer(textviewEditViewResponse), pakn[i].phanhoi, -1);
			sttInPAKNArr = i;

			gtk_widget_show(editAndViewWindow);
			return;
		}
	}
}

void saveEdit()
{
	GtkTextIter start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(textviewEditViewContent);

	strcpy(pakn[sttInPAKNArr].nguoiphananh, gtk_entry_get_text(entryEditViewName));
	strcpy(pakn[sttInPAKNArr].phanloai, gtk_entry_get_text(entryEditViewType));
	strcpy(pakn[sttInPAKNArr].email, gtk_entry_get_text(entryEmailEditView));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	strcpy(pakn[sttInPAKNArr].noidung, gtk_text_buffer_get_text(buffer, &start, &end, FALSE));
	gtk_widget_hide(editAndViewWindow);
	Display();
	Thongke();
}

void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
	glong id;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *trangthai;
	gboolean canEdit;

	model = gtk_tree_view_get_model(tree_view);

	if (!gtk_tree_model_get_iter(model, &iter, path))
		return;
	gtk_tree_model_get(model, &iter,
										 COL_ID, &id,
										 -1);
	gtk_tree_model_get(model, &iter,
										 COL_STATE, &trangthai,
										 -1);

	if ((strcmp(trangthai, "Mới ghi nhận") == 0) && (strcmp(gtk_widget_get_name(GTK_WIDGET(tree_view)), "quanly") == 0))
		canEdit = TRUE;
	else
		canEdit = FALSE;

	Popup(id, canEdit);
}

void on_searchEntry_activate(GtkEntry *searchEntry, gpointer user_data)
{
	Search();
}

void on_searchEntry_delete_text(GtkEditable *searchEntry, gint i, gpointer user_data)
{
	Display();
}

void DeleteID(long ID)
{
	int i;
	for (i = sum - 1; i >= 0; --i)
		if (pakn[i].id == ID)
			DeletePAKN(i);
	return;
}

void deletePAKNWithSelect()
{
	GtkTreeSelection *select = gtk_tree_view_get_selection(tableQuanLy);

	int countRowSelected = gtk_tree_selection_count_selected_rows(select);

	if (countRowSelected == 0)
	{
		showNotification(revealer, lblNotifi, "Chưa chọn dòng nào!", 1000);
	}
	else
	{
		GtkTreeIter iter;
		GtkTreeModel *model;
		gboolean valid;
		GList *_list = gtk_tree_selection_get_selected_rows(select, &model);

		glong id;

		int length = g_list_length(_list);
		for (int i = 0; i < length; i++)
		{
			//do some thing with l->data
			gtk_tree_model_get_iter(model, &iter, _list->data);
			gtk_tree_model_get(model, &iter,
												 COL_ID, &id,
												 -1);
			DeleteID(id);
			_list = g_list_next(_list);
		}

		Display();
		Thongke();
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
			sscanf(read, "%ld|%[^|]|%[^|]|%[^|]|%[^|]|%d|%[^|]|%d|%[^\n]%*c", &pakn[sum].id, pakn[sum].nguoiphananh, pakn[sum].email, pakn[sum].ngayphananh, pakn[sum].phanloai, &pakn[sum].solan, pakn[sum].noidung, &pakn[sum].trangthai, pakn[sum].phanhoi);
			sum++;
		}
	fclose(fp);
	return;
}

void login()
{
	char read[100], tmp[50];
	char name[50] = "Họ và tên: \0", birth[50] = "Ngày sinh: \0", role[30] = "Chức vụ: \0", unit[50] = "Đơn vị: \0";
	int result = 0;
	gchar *acc_entry = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(accountEntry)));
	gchar *pass_entry = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(passwordEntry)));

	if (strlen(acc_entry) == 0 || strlen(pass_entry) == 0)
	{
		gtk_label_set_text(errorLoginLabel, "Nhập đầy đủ tài khoản và mật khẩu");
		gtk_entry_set_text(GTK_ENTRY(accountEntry), "");
		gtk_entry_set_text(GTK_ENTRY(passwordEntry), "");
		return;
	}

	FILE *fp = fopen("account.txt", "r");

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

		sscanf(read, "%[^|]|%[^|]|", infoAccount.acc, infoAccount.pass);

		if ((strcmp(infoAccount.acc, acc_entry) == 0) && (strcmp(infoAccount.pass, pass_entry) == 0))
		{
			result = 1;

			sscanf(read, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]%*c", infoAccount.acc, infoAccount.pass, infoAccount.name, infoAccount.birthday, infoAccount.role, infoAccount.unit);
			strcat(name, infoAccount.name);
			strcat(birth, infoAccount.birthday);
			strcat(role, infoAccount.role);
			strcat(unit, infoAccount.unit);
			gtk_label_set_text(lbl_name, name);
			gtk_label_set_text(lbl_birthday, birth);
			gtk_label_set_text(lbl_level, role);
			gtk_label_set_text(lbl_workplace, unit);
			sprintf(tmp, "res/avatar/%s.jpg", infoAccount.acc);
			gtk_image_set_from_file(avatar, tmp);
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
	gtk_entry_set_text(GTK_ENTRY(accountEntry), "");
	gtk_entry_set_text(GTK_ENTRY(passwordEntry), "");

	fclose(fp);
	// g_free(acc_entry);
	// g_free(pass_entry);
	return;
}

void Display()
{
	GtkTreeIter iter;
	gtk_list_store_clear(listAllPAKN);
	int i = 0;
	for (i = sum - 1; i >= 0; --i)
		if (pakn[i].trangthai == 0)
		{
			gtk_list_store_append(listAllPAKN, &iter);

			/* Fill fields with some data */

			gtk_list_store_set(listAllPAKN, &iter,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_EMAIL, pakn[i].email,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_NUM, pakn[i].solan,
												 COL_CONTENT, pakn[i].noidung,
												 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
																																																				 : "Đã giải quyết",
												 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
												 -1);
		}
	for (i = sum - 1; i >= 0; --i)
		if (pakn[i].trangthai == 1)
		{
			gtk_list_store_append(listAllPAKN, &iter);

			/* Fill fields with some data */

			gtk_list_store_set(listAllPAKN, &iter,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_EMAIL, pakn[i].email,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_NUM, pakn[i].solan,
												 COL_CONTENT, pakn[i].noidung,
												 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
																																																				 : "Đã giải quyết",
												 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
												 -1);
		}
	for (i = sum - 1; i >= 0; --i)
		if (pakn[i].trangthai == 2)
		{
			gtk_list_store_append(listAllPAKN, &iter);

			/* Fill fields with some data */

			gtk_list_store_set(listAllPAKN, &iter,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_EMAIL, pakn[i].email,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_NUM, pakn[i].solan,
												 COL_CONTENT, pakn[i].noidung,
												 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
																																																				 : "Đã giải quyết",
												 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
												 -1);
		}
}

void Search()
{
	GtkTreeIter iter;
	gtk_list_store_clear(listAllPAKN);
	gchar *search = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(searchEntry)));
	if (strlen(search) == 0)
	{
		Display();
		return;
	}
	int i = 0, len = strlen(search);
	long devide = 1;
	for (i = len; i < 14; ++i)
		devide *= 10;
	for (i = sum - 1; i >= 0; --i)
	{
		if (pakn[i].id / devide == atol(search))
		{
			gtk_list_store_append(listAllPAKN, &iter);

			/* Fill fields with some data */

			gtk_list_store_set(listAllPAKN, &iter,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_EMAIL, pakn[i].email,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_NUM, pakn[i].solan,
												 COL_CONTENT, pakn[i].noidung,
												 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
																																																				 : "Đã giải quyết",
												 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
												 -1);
		}
	}
	gtk_entry_set_text(GTK_ENTRY(searchEntry), search);
}

void Thongke()
{
	int i;

	gtk_list_store_clear(listMoiGhiNhan);
	gtk_list_store_clear(listChuaGiaiQuyet);
	gtk_list_store_clear(listDaGiaiQuyet);

	GtkTreeIter iter1, iter2, iter3;

	for (i = sum - 1; i >= 0; --i)
	{
		switch (pakn[i].trangthai)
		{
		case 0:
			gtk_list_store_append(listMoiGhiNhan, &iter1);

			/* Fill fields with some data */

			gtk_list_store_set(listMoiGhiNhan, &iter1,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_EMAIL, pakn[i].email,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_NUM, pakn[i].solan,
												 COL_CONTENT, pakn[i].noidung,
												 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
																																																				 : "Đã giải quyết",
												 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
												 -1);
			break;

		case 1:
			gtk_list_store_append(listChuaGiaiQuyet, &iter2);

			/* Fill fields with some data */

			gtk_list_store_set(listChuaGiaiQuyet, &iter2,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_EMAIL, pakn[i].email,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_NUM, pakn[i].solan,
												 COL_CONTENT, pakn[i].noidung,
												 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
																																																				 : "Đã giải quyết",
												 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
												 -1);
			break;

		default:
			gtk_list_store_append(listDaGiaiQuyet, &iter3);

			/* Fill fields with some data */

			gtk_list_store_set(listDaGiaiQuyet, &iter3,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_EMAIL, pakn[i].email,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_NUM, pakn[i].solan,
												 COL_CONTENT, pakn[i].noidung,
												 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
																																																				 : "Đã giải quyết",
												 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
												 -1);
		}
	}
}

void AddPAKN()
{
	//Lấy mean từ TextView
	GtkTextIter start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(addContent);
	gchar *content;
	int year, month, day, hour, minute, second, i;
	PAKN new;

	gtk_text_buffer_get_bounds(buffer, &start, &end);
	content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	gchar *name = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entryAddName)));
	gchar *type = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entryAddType)));
	gchar *email = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entryAddEmail)));

	if (strlen(content) == 0 || strlen(name) == 0 || strlen(type) == 0)
	{
		showNotification(revealer1, lblNotifiAdd, "Nhập đầy đủ thông tin!!!", 1000);
		return;
	}

	strcpy(new.nguoiphananh, name);
	strcpy(new.email, email);
	strcpy(new.phanloai, type);
	strcpy(new.noidung, content);
	new.solan = 1;
	for (i = sum - 1; i >= 0; --i)
		if (!strcmp(name, pakn[i].nguoiphananh) && !strcmp(type, pakn[i].phanloai) && !strcmp(pakn[i].noidung, content) && !strcmp(email, pakn[i].email))
		{
			showNotification(revealer1, lblNotifiAdd, "Đã thêm trước đó, không cần thêm lại!", 1000);
			return;
		}
	strcpy(new.phanhoi, "Chưa có");
	new.trangthai = 0;
	GDateTime *date_time;
	gchar *dt_format;

	date_time = g_date_time_new_now_local();												// get local time
	dt_format = g_date_time_format(date_time, "%H:%M:%S %e/%m/%Y"); // 24hr time format
	sscanf(dt_format, "%d:%d:%d %d/%d/%d", &hour, &minute, &second, &day, &month, &year);
	new.id = second + minute * 100 + hour * 10000 + day * 1000000 + month * 100000000 + year * 10000000000;
	sprintf(new.ngayphananh, "%d/%d/%d", day, month, year);

	pakn[sum++] = new;
	Display();
	Thongke();
	showNotification(revealer, lblNotifi, "Thêm hoàn tất!", 1000);
	gtk_entry_set_text(GTK_ENTRY(entryAddName), "");
	gtk_entry_set_text(GTK_ENTRY(entryAddType), "");
	gtk_entry_set_text(GTK_ENTRY(entryAddEmail), "");
	gtk_text_buffer_set_text(gtk_text_view_get_buffer(addContent), "", -1);
	gtk_widget_hide(addWindow);
	// g_free(content);
	// g_free(name);
	// g_free(type);
	// g_free(dt_format);
}

long CreateID()
{
	GDateTime *date_time;
	gchar *dt_format;
	int year, month, day, hour, minute, second;

	date_time = g_date_time_new_now_local();												// get local time
	dt_format = g_date_time_format(date_time, "%H:%M:%S %e/%m/%Y"); // 24hr time format
	sscanf(dt_format, "%d:%d:%d %d/%d/%d", &hour, &minute, &second, &day, &month, &year);
	return second + minute * 100 + hour * 10000 + day * 1000000 + month * 100000000 + year * 10000000000;
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
	if (inSession)
	{
		ExportToFile();
		inSession = FALSE;
	}
	gtk_widget_show(loginWindow);
}

void DeletePAKN(int i)
{
	int j;
	for (j = i; j < sum - 1; ++j)
		pakn[j] = pakn[j + 1];
	sum--;
}

void CombinePAKN(int i, int j, int count)
{
	strcat(pakn[i].nguoiphananh, ", ");
	strcat(pakn[i].ngayphananh, ", ");
	strcat(pakn[i].email, ", ");
	strcat(pakn[i].email, pakn[j].email);
	strcat(pakn[i].nguoiphananh, pakn[j].nguoiphananh);
	strcat(pakn[i].ngayphananh, pakn[j].ngayphananh);
	pakn[i].solan += pakn[j].solan;
	pakn[i].id = CreateID() + count;
}

int Pos(long id)
{
	int i;
	for (i = 0; i < sum; ++i)
		if (pakn[i].id == id)
			return i;
}

void Combine()
{
	GtkTreeSelection *select = gtk_tree_view_get_selection(tableQuanLy);

	int countRowSelected = gtk_tree_selection_count_selected_rows(select);

	if (countRowSelected == 0)
	{
		int num = 1, i, j, count = 0;

		for (i = sum - 1; i >= 0; --i)
		{
			if (pakn[i].trangthai == 0)
				for (j = i + 1; j < sum; ++j)
					if (!pakn[j].trangthai && !strcmp(pakn[i].phanloai, pakn[j].phanloai) && !strcmp(pakn[i].noidung, pakn[j].noidung))
					{
						CombinePAKN(i, j, count);
						count++;
						DeletePAKN(j);
					}
		}
		Display();
		Thongke();
	}
	else
	{
		GtkTreeIter iter;
		GtkTreeModel *model;
		gboolean valid;
		GList *_list = gtk_tree_selection_get_selected_rows(select, &model);
		GList *Node = _list;

		glong id1, id2, del[10];
		gchar *dieukien1, *phanloai1, *phanloai2, *dieukien2;

		int length = g_list_length(_list), pos, delsum = 0;

		gtk_tree_model_get_iter(model, &iter, Node->data);
		gtk_tree_model_get(model, &iter,
											 COL_ID, &id2,
											 -1);
		gtk_tree_model_get(model, &iter,
											 COL_STATE, &dieukien1,
											 -1);
		gtk_tree_model_get(model, &iter,
											 COL_TYPE, &phanloai1,
											 -1);
		Node = g_list_next(Node);

		if (strcmp(dieukien1, "Mới ghi nhận") != 0)
		{
			showNotification(revealer, lblNotifi, "Vui lòng chọn những phản ánh, kiến nghị mới ghi nhận!", 1000);
			return;
		}

		for (int i = 1; i < length; i++)
		{
			//do some thing with l->data
			gtk_tree_model_get_iter(model, &iter, Node->data);
			gtk_tree_model_get(model, &iter,
												 COL_ID, &id2,
												 -1);
			gtk_tree_model_get(model, &iter,
												 COL_STATE, &dieukien2,
												 -1);
			gtk_tree_model_get(model, &iter,
												 COL_TYPE, &phanloai2,
												 -1);
			Node = g_list_next(Node);
			if (strcmp(phanloai1, phanloai2) != 0)
			{
				showNotification(revealer, lblNotifi, "Phản ánh kiến nghị không cùng loại!", 1000);
				return;
			}
			if (strcmp(dieukien1, "Mới ghi nhận") != 0)
			{
				showNotification(revealer, lblNotifi, "Vui lòng chọn những phản ánh, kiến nghị mới ghi nhận!", 1000);
				return;
			}
		}

		Node = g_list_first(_list);

		gtk_tree_model_get_iter(model, &iter, Node->data);
		gtk_tree_model_get(model, &iter,
											 COL_ID, &id1,
											 -1);
		gtk_tree_model_get(model, &iter,
											 COL_STATE, &dieukien1,
											 -1);

		pos = Pos(id1);

		Node = g_list_next(Node);

		for (int i = 1; i < length; i++)
		{
			//do some thing with l->data
			gtk_tree_model_get_iter(model, &iter, Node->data);
			gtk_tree_model_get(model, &iter,
												 COL_ID, &id2,
												 -1);
			gtk_tree_model_get(model, &iter,
												 COL_STATE, &dieukien2,
												 -1);
			CombinePAKN(pos, Pos(id2), 0);
			del[delsum++] = id2;
			Node = g_list_next(Node);
		}

		for (int i = 0; i < delsum; ++i)
			DeletePAKN(Pos(del[i]));

		Display();
		Thongke();
		g_free(dieukien1);
		g_free(dieukien2);
		g_free(phanloai1);
		g_free(phanloai2);

		gtk_tree_selection_unselect_all(select);
	}

	showNotification(revealer, lblNotifi, "Thành công", 1200);
}

void filterAListStore(GtkListStore *ls, gchar *quyFilter, gchar *namFilter, int type)
{
	GtkTreeIter iter;
	gtk_list_store_clear(ls);
	int i = 0;
	char day[3], month[3], year[5];
	int quarter = 0;
	for (i = sum - 1; i >= 0; --i)
	{
		sscanf(pakn[i].ngayphananh, "%[^/]/%[^/]/%s", day, month, year);

		quarter = atoi(month) / 3;
		if (atoi(month) % 3 > 0)
		{
			quarter++;
		}

		if ((g_utf8_collate(quyFilter, "Tất cả") == 0) && (g_utf8_collate(namFilter, "Tất cả") == 0) && (pakn[i].trangthai == type))
		{
			goto label;
		}
		else if ((g_utf8_collate(quyFilter, "Tất cả") == 0) && (atoi(year) == atoi(namFilter)) && (pakn[i].trangthai == type))
		{
			goto label;
		}
		else if ((g_utf8_collate(namFilter, "Tất cả") == 0) && (quarter == atoi(quyFilter)) && (pakn[i].trangthai == type))
		{
			goto label;
		}
		else if ((quarter == atoi(quyFilter)) && (atoi(year) == atoi(namFilter)) && (pakn[i].trangthai == type))
		{
			// gtk_list_store_append(ls, &iter);

			// gtk_list_store_set(ls, &iter,
			// 									 COL_ID, pakn[i].id,
			// 									 COL_NAME, pakn[i].nguoiphananh,
			// 									 COL_EMAIL, pakn[i].email,
			// 									 COL_DATE, pakn[i].ngayphananh,
			// 									 COL_TYPE, pakn[i].phanloai,
			// 									 COL_NUM, pakn[i].solan,
			// 									 COL_CONTENT, pakn[i].noidung,
			// 									 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
			// 																																																	 : "Đã giải quyết",
			// 									 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
			// 									 -1);

			goto label;
		}
		else
			continue;

	label:
		if (pakn[i].trangthai == type)
		{
			gtk_list_store_append(ls, &iter);

			gtk_list_store_set(ls, &iter,
												 COL_ID, pakn[i].id,
												 COL_NAME, pakn[i].nguoiphananh,
												 COL_EMAIL, pakn[i].email,
												 COL_DATE, pakn[i].ngayphananh,
												 COL_TYPE, pakn[i].phanloai,
												 COL_NUM, pakn[i].solan,
												 COL_CONTENT, pakn[i].noidung,
												 COL_STATE, (pakn[i].trangthai == 0) ? "Mới ghi nhận" : (pakn[i].trangthai == 1) ? "Chưa giải quyết"
																																																				 : "Đã giải quyết",
												 COL_RESPONSE, (pakn[i].trangthai == 2) ? pakn[i].phanhoi : "",
												 -1);
		}
	}
}

void filter()
{
	gchar *quy, *nam;
	GtkTreeModel *treeModel;

	if ((gtk_combo_box_text_get_active_text(quyComboText) == NULL) || (gtk_combo_box_text_get_active_text(namComboText) == NULL))
	{
		showNotification(revealer, lblNotifi, "Vui lòng chọn Quý và Năm để lọc dữ liệu!!!", 1200);
	}
	else
	{
		quy = gtk_combo_box_text_get_active_text(quyComboText);
		nam = gtk_combo_box_text_get_active_text(namComboText);

		gchar *currentStackAnalysis = gtk_stack_get_visible_child_name(stackAnalysis);

		if (strcmp(currentStackAnalysis, "moighinhan") == 0)
			filterAListStore(listMoiGhiNhan, quy, nam, 0);
		else if (strcmp(currentStackAnalysis, "chuagiaiquyet") == 0)
			filterAListStore(listChuaGiaiQuyet, quy, nam, 1);
		else
			filterAListStore(listDaGiaiQuyet, quy, nam, 2);
	}
}

void PrintAnalysisStatus(FILE *fp, int status, long min, long max)
{
	int i;
	if (status == 0)
		fprintf(fp, "Mói ghi nhận:\n");
	else if (status == 1)
		fprintf(fp, "Chưa phản hồi:\n");
	else
		fprintf(fp, "Đã phản hồi:\n");

	if (status != 2)
	{
		fprintf(fp, "ID|Người phản ánh|Email|Ngày phản ánh|Phân loại|Số lần phản ánh|Nội dung\n\n");
		for (int i = 0; i < sum; ++i)
			if ((pakn[i].id > min) && (pakn[i].id < max) && (pakn[i].trangthai == status))
				fprintf(fp, "%ld|%s|%s|%s|%s|%d|%s\n", pakn[i].id, pakn[i].nguoiphananh, pakn[i].email, pakn[i].ngayphananh, pakn[i].phanloai, pakn[i].solan, pakn[i].noidung);
	}
	if (status == 2)
	{
		fprintf(fp, "ID|Người phản ánh|Email|Ngày phản ánh|Phân loại|Số lần phản ánh|Nội dung|Phản hồi\n\n");
		for (int i = 0; i < sum; ++i)
			if ((pakn[i].id > min) && (pakn[i].id < max) && (pakn[i].trangthai == 2))
				fprintf(fp, "%ld|%s|%s|%s|%s|%d|%s|%s\n", pakn[i].id, pakn[i].nguoiphananh, pakn[i].email, pakn[i].ngayphananh, pakn[i].phanloai, pakn[i].solan, pakn[i].noidung, pakn[i].phanhoi);
	}
	fprintf(fp, "--------------------------------------------------------------------------\n");
}

void PrintAnalysis()
{
	gchar *quy, *nam;
	char fileAnalys[10];
	GtkTreeModel *treeModel;
	FILE *fp;
	int iquy, inam;
	long min, max;

	if ((gtk_combo_box_text_get_active_text(quyComboText) == NULL) || (gtk_combo_box_text_get_active_text(namComboText) == NULL))
	{
		showNotification(revealer, lblNotifi, "Vui lòng chọn Quý và Năm để lọc dữ liệu!!!", 1200);
	}
	else
	{
		quy = gtk_combo_box_text_get_active_text(quyComboText);
		nam = gtk_combo_box_text_get_active_text(namComboText);
		iquy = atoi(quy);
		inam = atoi(nam);
		strcpy(fileAnalys, nam);
		strcat(fileAnalys, quy);
		strcat(fileAnalys, ".txt");
	}

	if ((fp = fopen(fileAnalys, "w")) == NULL)
		return;
	min = inam * 10000000000 + (iquy * 3 - 2) * 100000000;
	max = min + 232000000;
	PrintAnalysisStatus(fp, 0, min, max);
	PrintAnalysisStatus(fp, 1, min, max);
	PrintAnalysisStatus(fp, 2, min, max);
	fclose(fp);
	showNotification(revealer, lblNotifi, "In thành công", 1200);

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
		logout();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnAdd") == 0)
	{
		gtk_window_present(addWindow);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnDel") == 0)
	{
		deletePAKNWithSelect();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnAddWordWindow") == 0)
	{
		AddPAKN();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnCloseAddWordWindow") == 0)
	{
		gtk_widget_hide(addWindow);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnCloseNotifi") == 0)
	{
		closeNotification(revealer);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnCloseNotifiAdd") == 0)
	{
		closeNotification(revealer1);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnCombine") == 0)
	{
		Combine();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnSendSuperior") == 0)
	{
		clearAllSMTP();
		Send();
		showNotification(revealer, lblNotifi, "Thành công", 1200);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnFilter") == 0)
	{
		filter();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnPrintAnalysis") == 0)
	{
		PrintAnalysis();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnCloseEditViewWindow") == 0)
	{
		gtk_widget_hide(editAndViewWindow);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnSave") == 0)
	{
		saveEdit();
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnSendResponse") == 0)
	{
		gtk_widget_show(dialogFileChooser);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnCloseLoadFile") == 0)
	{
		gtk_widget_hide(dialogFileChooser);
	}

	if (strcmp(gtk_widget_get_name(GTK_WIDGET(btn)), "btnLoadFile") == 0)
	{
		gtk_widget_hide(dialogFileChooser);
		gchar *fileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialogFileChooser));
		if (fileName == NULL)
		{
			showNotification(revealer, lblNotifi, "Chưa chọn file!!!", 1500);
		}
		else
		{
			// connectSMTP();
			SendReply(fileName);
			showNotification(revealer, lblNotifi, "Thành công", 1200);
			// closeSMTP();
		}
	}
}

void printToFile(FILE *fp, PAKN pakn)
{
	char *ptr1, *ptr2;
	char name[30], date[20];
	int num = 1, i;

	fprintf(fp, "Số lần phản ánh: %d\n", pakn.solan);
	fprintf(fp, "Người phản ánh\tNgày phản ánh:\n");

	for (i = 0; i < strlen(pakn.nguoiphananh); ++i)
		if (pakn.nguoiphananh[i] == ',')
			num++;

	ptr1 = pakn.nguoiphananh;
	ptr2 = pakn.ngayphananh;

	for (i = 0; i < num; ++i)
	{
		if (i != num - 1)
		{
			sscanf(ptr1, "%[^,]", name);
			sscanf(ptr2, "%[^,]", date);
			ptr1 = strchr(ptr1, ',') + 2;
			ptr2 = strchr(ptr2, ',') + 2;
		}
		else
		{
			strcpy(name, ptr1);
			strcpy(date, ptr2);
		}
		fprintf(fp, "%s\t%s\n", name, date);
	}

	fprintf(fp, "Phân loại: %s\nNội dung: %s\n", pakn.phanloai, pakn.noidung);
	fprintf(fp, "-------------------------------------------\n");
	return;
}

void Send()
{
	long id = CreateID();
	char send[22] = "send";
	int i;
	char s[1000], superior_mail[50];

	FILE *f = fopen("superior_mail.txt", "r");

	if (f == NULL)
	{
		return;
	}

	fscanf(f, "%[^\n]%*c", superior_mail);
	fclose(f);

	for (i = 0; i < 14; ++i)
	{
		send[17 - i] = id % 10 + '0';
		id /= 10;
	}
	strcpy(send + 18, ".txt");
	send[22] = '\0';

	FILE *fp = fopen(send, "w+");

	if (fp == NULL)
		return;

	Combine();

	for (i = sum - 1; i >= 0; --i)
	{
		if (pakn[i].trangthai == 0)
		{
			printToFile(fp, pakn[i]);
			pakn[i].trangthai = 1;
		}
	}
	fclose(fp);

	Display();
	Thongke();

	sendMail(superior_mail, "Cấp trên", "Những kiến nghị cần giải quyết", "", send);

	// remove(send);
}

void replyUpdate(long id, char *reply)
{
	char name[50], date[30], email[30], *ptr1, *ptr2, *ptr3, mail[1000], noidung[200];
	int num = 0, i;

	for (i = sum - 1; i >= 0; --i)
		if (pakn[i].id == id && pakn[i].trangthai == 1)
		{
			strcpy(pakn[i].phanhoi, reply);
			strcpy(noidung, pakn[i].noidung);
			pakn[i].trangthai = 2;
			ptr1 = pakn[i].nguoiphananh;
			ptr2 = pakn[i].ngayphananh;
			ptr3 = pakn[i].email;
			num = pakn[i].solan;
			break;
		}
	if (num == 0)
		return;
	for (i = 0; i < num; ++i)
	{
		strcpy(mail, "");
		if (i != num - 1)
		{
			sscanf(ptr1, "%[^,]", name);
			sscanf(ptr2, "%[^,]", date);
			sscanf(ptr3, "%[^,]", email);
			ptr1 = strchr(ptr1, ',') + 2;
			ptr2 = strchr(ptr2, ',') + 2;
			ptr3 = strchr(ptr3, ',') + 2;
		}
		else
		{
			strcpy(name, ptr1);
			strcpy(date, ptr2);
			strcpy(email, ptr3);
		}
		sprintf(mail, "Gửi công dân: %s\nVề kiến nghị ngày: %s\nNội dung: %s\n\nPhản hồi của nhà chức trách: %s\n------------------------------------------\nNếu có bất kỳ ý kiến thắc mắc, khiếu nại nào, vui lòng liên hệ phường để được hỗ trợ.", name, date, noidung, reply);
		clearAllSMTP();
		sendMail(email, "Công dân", "Phản hồi PAKN", mail, "");
	}
}
void SendReply(char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
	{
		showNotification(revealer, lblNotifi, "Đọc file lỗi", 1200);
		return;
	}
	long id;
	int i;
	char reply[200], read[200];

	while (!feof(fp))
	{
		if (fgets(read, sizeof(read), fp) != NULL)
		{
			sscanf(read, "%ld|%[^\n]\n", &id, reply);
			replyUpdate(id, reply);
		}
	}

	Display();
	Thongke();
}

void ExportToFile()
{
	FILE *fp = fopen("pakn.txt", "w");
	if (fp == NULL)
		return;

	fprintf(fp, "ID|Người phản ánh|Email|Ngày phản ánh|Phân loại|Số lần phản ánh|Nội dung|Trạng thái|Phản hồi //Trang thai: [0] moi them, [1] chua Phản hồi, [2], da Phản hồi\n");

	for (int i = 0; i < sum; ++i)
		fprintf(fp, "%ld|%s|%s|%s|%s|%d|%s|%d|%s\n", pakn[i].id, pakn[i].nguoiphananh, pakn[i].email, pakn[i].ngayphananh, pakn[i].phanloai, pakn[i].solan, pakn[i].noidung, pakn[i].trangthai, pakn[i].phanhoi);
	fclose(fp);
	g_free(pakn);
	return;
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
	closeSMTP();
	if (inSession)
	{
		ExportToFile();
		inSession = FALSE;
	}

	gtk_main_quit();
}

int main(int argc,
				 char *argv[])
{
	connectSMTP();
	///////////////////////////////////////////////////////////////////
	GtkBuilder *builder;
	// Khởi tạo GTK
	gtk_init(&argc, &argv);

	// Load giao diện từ file .glade vào biến builder
	builder = gtk_builder_new_from_file("glade/KTPM.glade");

	// Gán các widget được lấy từ file .glade
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
	entryAddName = GTK_ENTRY(gtk_builder_get_object(builder, "entryAddName"));
	entryAddType = GTK_ENTRY(gtk_builder_get_object(builder, "entryAddType"));
	addContent = GTK_WIDGET(gtk_builder_get_object(builder, "addContent"));
	revealer = GTK_REVEALER(gtk_builder_get_object(builder, "revealer"));
	revealer1 = GTK_REVEALER(gtk_builder_get_object(builder, "revealer1"));
	revealer2 = GTK_REVEALER(gtk_builder_get_object(builder, "revealer2"));
	lblNotifi = GTK_LABEL(gtk_builder_get_object(builder, "lblNotifi"));
	lblNotifiAdd = GTK_LABEL(gtk_builder_get_object(builder, "lblNotifiAdd"));
	searchEntry = GTK_ENTRY(gtk_builder_get_object(builder, "searchEntry"));
	btnCombine = GTK_BUTTON(gtk_builder_get_object(builder, "btnCombine"));
	btnSendSuperior = GTK_BUTTON(gtk_builder_get_object(builder, "btnSendSuperior"));
	btnFilter = GTK_BUTTON(gtk_builder_get_object(builder, "btnFilter"));
	btnPrintAnalysis = GTK_BUTTON(gtk_builder_get_object(builder, "btnPrintAnalysis"));
	quyComboText = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "quyComboText"));
	namComboText = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "namComboText"));
	quanly = GTK_TREE_VIEW(gtk_builder_get_object(builder, "quanly"));
	stackAnalysis = GTK_STACK(gtk_builder_get_object(builder, "stackAnalysis"));
	editAndViewWindow = GTK_WINDOW(gtk_builder_get_object(builder, "editAndViewWindow"));
	entryEditViewName = GTK_ENTRY(gtk_builder_get_object(builder, "entryEditViewName"));
	entryEditViewType = GTK_ENTRY(gtk_builder_get_object(builder, "entryEditViewType"));
	textviewEditViewContent = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "textviewEditViewContent"));
	lblEditViewResponse = GTK_LABEL(gtk_builder_get_object(builder, "lblEditViewResponse"));
	textviewEditViewResponse = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "textviewEditViewResponse"));
	btnSave = GTK_BUTTON(gtk_builder_get_object(builder, "btnSave"));
	btnCloseEditViewWindow = GTK_BUTTON(gtk_builder_get_object(builder, "btnCloseEditViewWindow"));
	lblNotifiEditView = GTK_LABEL(gtk_builder_get_object(builder, "lblNotifiEditView"));
	scrollForResponse = GTK_SCROLLED_WINDOW(gtk_builder_get_object(builder, "scrollForResponse"));
	entryEmailEditView = GTK_ENTRY(gtk_builder_get_object(builder, "entryEmailEditView"));
	entryAddEmail = GTK_ENTRY(gtk_builder_get_object(builder, "entryAddEmail"));
	entryNumPAKN = GTK_ENTRY(gtk_builder_get_object(builder, "entryNumPAKN"));
	btnSendResponse = GTK_BUTTON(gtk_builder_get_object(builder, "btnSendResponse"));
	btnLoadFile = GTK_BUTTON(gtk_builder_get_object(builder, "btnLoadFile"));
	btnCloseLoadFile = GTK_BUTTON(gtk_builder_get_object(builder, "btnCloseLoadFile"));
	dialogFileChooser = GTK_FILE_CHOOSER_DIALOG(gtk_builder_get_object(builder, "dialogFileChooser"));

	sortableQuanLy = GTK_TREE_SORTABLE(listAllPAKN);
	gtk_tree_sortable_set_sort_func(sortableQuanLy, COL_STATE, sort_iter_compare_func,
																	GINT_TO_POINTER(COL_STATE), NULL);
	// gtk_tree_sortable_set_sort_column_id (sortableQuanLy,
	//                                     	COL_STATE,
	//                                     	GTK_SORT_ASCENDING);

	gchar tmp[10];

	gtk_combo_box_text_append_text(quyComboText, "Tất cả");
	for (int i = 1; i <= 4; i++)
	{
		sprintf(tmp, "%d", i);
		gtk_combo_box_text_append_text(quyComboText, tmp);
	}

	gtk_combo_box_text_append_text(namComboText, "Tất cả");
	for (int i = 2021; i >= 2000; i--)
	{
		sprintf(tmp, "%d", i);
		gtk_combo_box_text_append_text(namComboText, tmp);
	}

	// set_css();
	// Cấp phát bộ nhớ động
	pakn = malloc(INIT_PAKN * sizeof(PAKN));

	// Gán time out cho progressBar
	g_timeout_add(500, close_splash_screen, progressBar);
	g_timeout_add_seconds(1, (GSourceFunc)timer_handler, lbl_time);

	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(builder);

	gtk_widget_show(window_splash_screen);

	gtk_main();
}
