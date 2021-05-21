
/* Close the splash screen */
gboolean close_screen(gpointer data)
{
	GtkWidget *progress_bar = data;

	/*Get the current progress*/
	// gdouble fraction;
	// fraction = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(progress_bar));

	// /*Increase the bar by 10% each time this function is called*/
	// fraction += 0.4;

	// /*Fill in the bar with the new fraction*/
	// gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), fraction);

	// if (fraction < 1.0)
	// 	return TRUE;

	gtk_widget_hide(window_splash_screen);

	gtk_widget_destroy((GtkWidget *)data);
	isShowedSplashScreen = TRUE;
	// gtk_main_quit();

	return (FALSE);
}

int Show_Splash_Screen(GtkApplication *app)
{
	window_splash_screen = GTK_WINDOW(gtk_builder_get_object(builder, "splash-screen"));
	gtk_window_set_application (GTK_WINDOW (window_splash_screen), app);

	gtk_widget_show(GTK_WIDGET(window_splash_screen));
	//gtk_main();
	//g_timeout_add(time, close_screen, GTK_PROGRESS_BAR(progressBar));
	return 0;
}
