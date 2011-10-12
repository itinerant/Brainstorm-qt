#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QtSql>
#include <QTextListFormat>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    // File menu
	void newCategory();
    void renameCategory();
	void deleteCategory();
	void newNote();
    void renameNote();
	void saveNote();
	void deleteNote();
	void quit();
	void closeEvent(QCloseEvent*);

	// Edit menu
	void findAndReplace();

	// Format menu
	void bold();
	void italic();
	void underline();
	void strikethrough();
	void bulletList();
	void numberList();
	void showColors();
	void showFonts();
	void increaseFont();
	void decreaseFont();
	void resetFont();
	void alignLeft();
	void alignCenter();
	void alignJustify();
	void alignRight();
	void increaseIndent();
	void decreaseIndent();

	// View menu
	void nextCategory();
	void previousCategory();
	void nextNote();
	void previousNote();

	// Help menu
	void aboutQt();
	void aboutBrainstorm();

	// Application
	void updateMenus();
	void setFont(QString);
	void setFontSize(QString);
	void setFontColor(int);

	void updateCategoryList();
	void updateNoteList(QModelIndex index);
	void updateNoteText(QModelIndex index);

	void iconActivated(QSystemTrayIcon::ActivationReason);
	void restoreBrainstorm();
private:	
    Ui::MainWindow *ui;

	QAction *minimizeAction;
	QAction *maximizeAction;
	QAction *restoreAction;
	QAction *quitAction;
	QSystemTrayIcon *trayIcon;
	QMenu *trayIconMenu;

	QSqlDatabase db;
	QSqlTableModel *categoriesModel;
	QSqlTableModel *notesModel;
	int currentNote;

	void saveNoteText();
	void toggleList(QTextListFormat::Style);
	bool eventFilter(QObject *object, QEvent *event);
	void createActions();
	void createTrayIcon();
};

#endif // MAINWINDOW_H
