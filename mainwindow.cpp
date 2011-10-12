#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QColorDialog>
#include <QColor>
#include <QFontDialog>
#include <QTextList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	// open database connection
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/Users/jdoud/dev/brainstorm.sqlite");
	if(!db.open())
	{
		qDebug() << db.lastError();
		qFatal("Failed to connect.");
	}

	// setup UI
    ui->setupUi(this);
	ui->toolBar->addWidget(ui->comboFonts);
	ui->toolBar->addWidget(ui->comboFontSizes);
	ui->toolBar->addWidget(ui->comboColors);

	// set text editor defaults
	ui->textNote->document()->setIndentWidth(20);
	ui->textNote->setTabStopWidth(20);
	ui->textNote->setTabChangesFocus(false);
	ui->actionIncrease_Indent->setShortcut(Qt::Key_Tab);
	ui->actionDecrease_Indent->setShortcut(Qt::Key_Backtab);

	// setup comboColors
	QPixmap pix(16, 16);
	pix.fill(Qt::white);
	ui->comboColors->addItem(pix, "");
	pix.fill(Qt::black);
	ui->comboColors->addItem(pix, "");
	pix.fill(Qt::red);
	ui->comboColors->addItem(pix, "");
	pix.fill(Qt::blue);
	ui->comboColors->addItem(pix, "");
	pix.fill(Qt::darkGreen);
	ui->comboColors->addItem(pix, "");
	pix.fill(Qt::gray);
	ui->comboColors->addItem(pix, "");


	// create system tray icon
	createActions();
	createTrayIcon();

	// create models
    categoriesModel = new QSqlTableModel();
	categoriesModel->setTable("categories");
	categoriesModel->setSort(1, Qt::AscendingOrder);
	categoriesModel->select();
	ui->listCategories->setModel(categoriesModel);
	ui->listCategories->setModelColumn(1);

    notesModel = new QSqlTableModel();
	notesModel->setTable("notes");
	ui->listNotes->setModel(notesModel);
	ui->listNotes->setModelColumn(2);

    // set splitter size
    QList<int> sizes;
    sizes << 230 << 150;
    ui->splitterLists->setSizes(sizes);
    sizes.clear();
    sizes << 230 << 600;
    ui->splitterNote->setSizes(sizes);

    // connect File menu slots
    connect(ui->actionNew_Category, SIGNAL(triggered()), this, SLOT(newCategory()));
    connect(ui->actionRename_Category, SIGNAL(triggered()), this, SLOT(renameCategory()));
    connect(ui->actionDelete_Category, SIGNAL(triggered()), this, SLOT(deleteCategory()));
    connect(ui->actionNew_Note, SIGNAL(triggered()), this, SLOT(newNote()));
    connect(ui->actionRename_Note, SIGNAL(triggered()), this, SLOT(renameNote()));
    connect(ui->actionSave_Note, SIGNAL(triggered()), this, SLOT(saveNote()));
    connect(ui->actionDelete_Note, SIGNAL(triggered()), this, SLOT(deleteNote()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(quit()));
    // connect Edit menu slots	
    connect(ui->actionFind_Replace, SIGNAL(triggered()), this, SLOT(findAndReplace()));
    // connect Format menu slots
    connect(ui->actionBold, SIGNAL(triggered()), this, SLOT(bold()));
    connect(ui->actionItalic, SIGNAL(triggered()), this, SLOT(italic()));
    connect(ui->actionUnderline, SIGNAL(triggered()), this, SLOT(underline()));
    connect(ui->actionStrikethrough, SIGNAL(triggered()), this, SLOT(strikethrough()));
    connect(ui->actionBullet_List, SIGNAL(triggered()), this, SLOT(bulletList()));
    connect(ui->actionNumber_List, SIGNAL(triggered()), this, SLOT(numberList()));
    connect(ui->actionIncrease_Indent, SIGNAL(triggered()), this, SLOT(increaseIndent()));
    connect(ui->actionDecrease_Indent, SIGNAL(triggered()), this, SLOT(decreaseIndent()));
    connect(ui->actionShow_Colors, SIGNAL(triggered()), this, SLOT(showColors()));
    connect(ui->actionShow_Fonts, SIGNAL(triggered()), this, SLOT(showFonts()));
    connect(ui->actionIncrease_Font, SIGNAL(triggered()), this, SLOT(increaseFont()));
    connect(ui->actionDecrease_Font, SIGNAL(triggered()), this, SLOT(decreaseFont()));
    connect(ui->actionReset_Font, SIGNAL(triggered()), this, SLOT(resetFont()));
    connect(ui->actionAlign_Left, SIGNAL(triggered()), this, SLOT(alignLeft()));
    connect(ui->actionAlign_Center, SIGNAL(triggered()), this, SLOT(alignCenter()));
    connect(ui->actionAlign_Right, SIGNAL(triggered()), this, SLOT(alignRight()));
    connect(ui->actionAlign_Justify, SIGNAL(triggered()), this, SLOT(alignJustify()));
    // connect View menu slots
    connect(ui->actionHide_Window, SIGNAL(triggered()), this, SLOT(hide()));
    connect(ui->actionPrevious_Category, SIGNAL(triggered()), this, SLOT(previousCategory()));
    connect(ui->actionNext_Category, SIGNAL(triggered()), this, SLOT(nextCategory()));
    connect(ui->actionPrevious_Note, SIGNAL(triggered()), this, SLOT(previousNote()));
    connect(ui->actionNext_Note, SIGNAL(triggered()), this, SLOT(nextNote()));
    // connect Help menu slots
    connect(ui->actionAbout_Brainstorm, SIGNAL(triggered()), this, SLOT(aboutBrainstorm()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(aboutQt()));
	// connect application slots
	connect(ui->textNote, SIGNAL(cursorPositionChanged()), this, SLOT(updateMenus()));
	connect(ui->textNote, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this, SLOT(updateMenus()));
    connect(ui->comboFonts, SIGNAL(activated(QString)), this, SLOT(setFont(QString)));
    connect(ui->comboFontSizes, SIGNAL(activated(QString)), this, SLOT(setFontSize(QString)));
    connect(ui->comboColors, SIGNAL(activated(int)), this, SLOT(setFontColor(int)));
	// connect category list slots
	connect(ui->listCategories->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateNoteList(QModelIndex)));
	// connect note list slots
	connect(ui->listNotes->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(updateNoteText(QModelIndex)));
	// connect text slots
	ui->textNote->installEventFilter((this));
	// connect system tray icon
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

	// initialize default data
	ui->listCategories->selectionModel()->setCurrentIndex(categoriesModel->index(0, 1), QItemSelectionModel::SelectCurrent);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete categoriesModel;
    delete notesModel;

    db.close();
    QSqlDatabase::removeDatabase(db.database().databaseName());
}

//////////////////////////////
// File Menu actions
//////////////////////////////
void MainWindow::newCategory()
{
	QSqlRecord r = categoriesModel->record();
	r.setNull("id");
	r.setValue("name", "A New Category");
	categoriesModel->insertRecord(0, r);
	categoriesModel->submitAll();
	ui->listCategories->selectionModel()->setCurrentIndex(categoriesModel->index(0, 1), QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
}
void MainWindow::renameCategory()
{
    ui->listCategories->setFocus();
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(ui->listCategories, &keyEvent);
}
void MainWindow::deleteCategory()
{
	if(notesModel->rowCount() > 0)
	{
		QMessageBox::warning(this, "Category Deletion Cancelled", "All notes within a category must be deleted\nbefore a category can be deleted.");
	}
	else
	{
		if(QMessageBox::question(this, "Delete Category", "Do you want to delete the category: "
								 + ui->listCategories->selectionModel()->currentIndex().data().toString() + "?",
								 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
		{
			categoriesModel->removeRow(ui->listCategories->selectionModel()->currentIndex().row());
			categoriesModel->submitAll();
			ui->listCategories->selectionModel()->setCurrentIndex(categoriesModel->index(0, 1), QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
		}
	}
}
void MainWindow::newNote()
{
	QSqlRecord r = notesModel->record();
	r.setNull("id");
	r.setValue("category_id", categoriesModel->record(ui->listCategories->selectionModel()->currentIndex().row()).value("id").toInt());
	r.setValue("name", "A New Note");
	notesModel->insertRecord(0, r);
    notesModel->submitAll();

    // find location of new note
    int index = 0;
    while(ui->listNotes->selectionModel()->currentIndex().data().toString() != "A New Note")
    {
        QMessageBox::information(this, "", ui->listNotes->selectionModel()->currentIndex().data().toString());
        ui->listNotes->selectionModel()->setCurrentIndex(notesModel->index(index, 1), QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
        index++;
    }


    //renameNote();
}
void MainWindow::renameNote()
{
    ui->listNotes->setFocus();
    QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(ui->listNotes, &keyEvent);
}
void MainWindow::saveNote() { saveNoteText(); }
void MainWindow::deleteNote()
{
	if(QMessageBox::question(this, "Delete Note", "Do you want to delete the note: "
							 + ui->listNotes->selectionModel()->currentIndex().data().toString() + "?",
							 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	{
		notesModel->removeRow(ui->listNotes->selectionModel()->currentIndex().row());
		notesModel->submitAll();
		ui->listNotes->selectionModel()->setCurrentIndex(notesModel->index(0, 1), QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
	}
}

void MainWindow::quit() { qApp->quit(); }

void MainWindow::closeEvent( QCloseEvent* e)
{
    e->ignore();
	saveNoteText();
    this->quit();
}

//////////////////////////////
// Edit Menu actions
//////////////////////////////
void MainWindow::findAndReplace() { //findDlg->show();
}

//////////////////////////////
// Format Menu actions
//////////////////////////////
void MainWindow::bold()
{
	if(ui->textNote->fontWeight() == QFont::Normal)
		ui->textNote->setFontWeight(QFont::Bold);
	else
		ui->textNote->setFontWeight(QFont::Normal);
}

void MainWindow::italic() { ui->textNote->setFontItalic(!ui->textNote->fontItalic()); }
void MainWindow::underline() { ui->textNote->setFontUnderline(!ui->textNote->fontUnderline()); }
void MainWindow::strikethrough()
{
	QTextCharFormat fmt = ui->textNote->currentCharFormat();
	fmt.setFontStrikeOut(!fmt.fontStrikeOut());
	ui->textNote->setCurrentCharFormat(fmt);
}

void MainWindow::bulletList()
{
	toggleList(QTextListFormat::ListDisc);
}

void MainWindow::numberList()
{
	toggleList(QTextListFormat::ListDecimal);
}

void MainWindow::toggleList(QTextListFormat::Style style)
{
	QTextCursor cursor = ui->textNote->textCursor();
	QTextBlockFormat blockFmt = cursor.blockFormat();
	QTextListFormat listFmt;

	bool list = (cursor.currentList() != 0);

	// change style if list exists and is a different style
	if(list && cursor.currentList()->format().style() != style)
	{
		listFmt.setStyle(style);
		cursor.currentList()->setFormat(listFmt);
	}
	// remove list if exists and matches style
	else if(list&& cursor.currentList()->format().style() == style)
	{
		cursor.currentList()->removeItem(0);
		blockFmt = ui->textNote->textCursor().blockFormat();
		cursor = ui->textNote->textCursor();
		blockFmt.setIndent(0);
		cursor.setBlockFormat(blockFmt);
	// create list if not exists
	}
	else
	{
		cursor.beginEditBlock();
		if (cursor.currentList()) {
			listFmt = cursor.currentList()->format();
		} else {
			listFmt.setIndent(blockFmt.indent() + 1);
			blockFmt.setIndent(0);
			cursor.setBlockFormat(blockFmt);
		}

		listFmt.setStyle(style);
		cursor.createList(listFmt);
		cursor.endEditBlock();
	}
	updateMenus();
}

void MainWindow::increaseIndent()
{
	QTextBlockFormat blockFmt = ui->textNote->textCursor().blockFormat();
	QTextCursor cursor = ui->textNote->textCursor();

	blockFmt.setIndent(blockFmt.indent()+1);
	cursor.setBlockFormat(blockFmt);
}

void MainWindow::decreaseIndent()
{
	QTextBlockFormat blockFmt = ui->textNote->textCursor().blockFormat();
	QTextCursor cursor = ui->textNote->textCursor();

	if(blockFmt.indent() > 0)
		blockFmt.setIndent(blockFmt.indent()-1);
	else
		blockFmt.setIndent(0);
	cursor.setBlockFormat(blockFmt);
}

void MainWindow::showFonts()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, ui->textNote->currentFont(), this);
	if(ok)
		ui->textNote->setCurrentFont(font);
}
void MainWindow::showColors()
{
	QColor color = QColorDialog::getColor(ui->textNote->textColor(), this);
	if(color.isValid())
		ui->textNote->setTextColor(color);
}

void MainWindow::setFont(QString font)
{
	ui->textNote->setFontFamily(font);
}

void MainWindow::setFontSize(QString size)
{
	ui->textNote->setFontPointSize(size.toDouble());
}

void MainWindow::setFontColor(int color)
{
	switch (color)
	{
		case 1:
			ui->textNote->setTextColor(Qt::black);
			break;
		case 2:
			ui->textNote->setTextColor(Qt::red);
			break;
		case 3:
			ui->textNote->setTextColor(Qt::blue);
			break;
		case 4:
			ui->textNote->setTextColor(Qt::darkGreen);
			break;
		case 5:
			ui->textNote->setTextColor(Qt::gray);
			break;
	}
}

void MainWindow::increaseFont()
{
    disconnect(ui->comboFontSizes, SIGNAL(triggered(QString)), this, SLOT(setFontSize(QString)));

	// step .5 up to 8
	// step 1.0 up to 14
	// step 2.0 up to 30
	// step 6 over 30
	double increment = 0.0f;
	if (ui->textNote->currentFont().pointSizeF() < 8)
		increment = 0.5f;
	else if (ui->textNote->currentFont().pointSizeF() < 14)
		increment = 1.0f;
	else if (ui->textNote->currentFont().pointSizeF() <= 29)
		increment = 2.0f;
	else
		increment = 6.0f;

	ui->textNote->setFontPointSize(ui->textNote->currentFont().pointSizeF() + increment);

    connect(ui->comboFontSizes, SIGNAL(triggered(QString)), this, SLOT(setFontSize(QString)));
}
void MainWindow::decreaseFont()
{
    disconnect(ui->comboFontSizes, SIGNAL(triggered(QString)), this, SLOT(setFontSize(QString)));

	// step .5 up to 8
	// step 1.0 up to 14
	// step 2.0 up to 30
	// step 6 over 32
	double decrement = 0.0f;
	if (ui->textNote->currentFont().pointSizeF() <= 8)
		decrement = 0.5f;
	else if (ui->textNote->currentFont().pointSizeF() <= 14)
		decrement = 1.0f;
	else if (ui->textNote->currentFont().pointSizeF() <= 32)
		decrement = 2.0f;
	else
		decrement = 6.0f;

	ui->textNote->setFontPointSize(ui->textNote->currentFont().pointSizeF() - decrement);

    connect(ui->comboFontSizes, SIGNAL(triggered(QString)), this, SLOT(setFontSize(QString)));
}

void MainWindow::resetFont()
{
	int pos = ui->textNote->textCursor().position();
	ui->textNote->selectAll();
	ui->textNote->setCurrentFont(QFont("Calibri", 11));
	ui->textNote->setTextColor(Qt::black);
	QTextCursor textCursor = ui->textNote->textCursor();
	textCursor.clearSelection();
	textCursor.setPosition(pos);
	ui->textNote->setTextCursor( textCursor );
}

void MainWindow::alignLeft() { ui->textNote->setAlignment(Qt::AlignLeft); updateMenus(); }
void MainWindow::alignCenter() { ui->textNote->setAlignment(Qt::AlignCenter); updateMenus(); }
void MainWindow::alignJustify() { ui->textNote->setAlignment(Qt::AlignJustify); updateMenus(); }
void MainWindow::alignRight() { ui->textNote->setAlignment(Qt::AlignRight); updateMenus(); }

//////////////////////////////
// View Menu actions
//////////////////////////////
void MainWindow::nextCategory()
{
	if(ui->listCategories->selectionModel()->currentIndex().row() < ui->listCategories->selectionModel()->model()->rowCount()-1)
	{
		saveNoteText();
		int row = ui->listCategories->selectionModel()->currentIndex().row()+1;
		ui->listCategories->selectionModel()->setCurrentIndex(categoriesModel->index(row, 1), QItemSelectionModel::SelectCurrent);
		updateMenus();
		ui->textNote->setFocus();
	}
}
void MainWindow::previousCategory()
{
	if(ui->listCategories->selectionModel()->currentIndex().row() >0)
	{
		saveNoteText();
		int row = ui->listCategories->selectionModel()->currentIndex().row()-1;
		ui->listCategories->selectionModel()->setCurrentIndex(categoriesModel->index(row, 1), QItemSelectionModel::SelectCurrent);
		updateMenus();
		ui->textNote->setFocus();
	}
}
void MainWindow::nextNote()
{
	if(currentNote < ui->listNotes->selectionModel()->model()->rowCount()-1)
	{
		saveNoteText();
		int row = currentNote+1;
		ui->listNotes->selectionModel()->setCurrentIndex(notesModel->index(row, 2), QItemSelectionModel::SelectCurrent);
		updateMenus();
		ui->textNote->setFocus();
	}
}
void MainWindow::previousNote()
{
	if(currentNote > 0)
	{
		saveNoteText();
		int row = currentNote-1;
		ui->listNotes->selectionModel()->setCurrentIndex(notesModel->index(row, 2), QItemSelectionModel::SelectCurrent);
		updateMenus();
		ui->textNote->setFocus();
	}
}

//////////////////////////////
// Help Menu actions
//////////////////////////////
void MainWindow::aboutBrainstorm() { QMessageBox::about(this, "Brainstorm", "A small utility to keep track of random thoughts."); }
void MainWindow::aboutQt() { QMessageBox::aboutQt(this, "About Qt"); }

//////////////////////////////
// Application slots
//////////////////////////////
void MainWindow::updateMenus()
{
	// paste menu
	ui->actionPaste->setEnabled(ui->textNote->canPaste());

	// current font style
	ui->actionBold->setChecked(ui->textNote->fontWeight() == QFont::Bold);
	ui->actionItalic->setChecked(ui->textNote->fontItalic());
	ui->actionUnderline->setChecked(ui->textNote->fontUnderline());
	ui->actionStrikethrough->setChecked(ui->textNote->currentCharFormat().fontStrikeOut());

	// current font
	ui->comboFonts->setCurrentIndex(ui->comboFonts->findText(ui->textNote->currentFont().family()));
	ui->comboFontSizes->setCurrentIndex(ui->comboFontSizes->findText(QString::number(ui->textNote->currentFont().pointSizeF())));
	QString name = ui->textNote->currentCharFormat().foreground().color().name();
	if(name == "#000000") // Qt::black
		ui->comboColors->setCurrentIndex(1);
	else if(name == "#ff0000") // Qt::red
		ui->comboColors->setCurrentIndex(2);
	else if(name == "#0000ff") // Qt::blue
		ui->comboColors->setCurrentIndex(3);
	else if(name == "#008000") // Qt::darkGreen
		ui->comboColors->setCurrentIndex(4);
	else if(name == "#a0a0a4") // Qt::gray
		ui->comboColors->setCurrentIndex(5);
	else // all other colors
		ui->comboColors->setCurrentIndex(0);

	// current alignment
	ui->actionAlign_Left->setChecked(ui->textNote->alignment() == Qt::AlignLeft);
	ui->actionAlign_Center->setChecked(ui->textNote->alignment() == Qt::AlignCenter);
	ui->actionAlign_Right->setChecked(ui->textNote->alignment() == Qt::AlignRight);
	ui->actionAlign_Justify->setChecked(ui->textNote->alignment() == Qt::AlignJustify);

	// current list style
	if(ui->textNote->textCursor().currentList())
	{
		ui->actionBullet_List->setChecked(ui->textNote->textCursor().currentList()->format().style() == QTextListFormat::ListDisc);
		ui->actionNumber_List->setChecked(ui->textNote->textCursor().currentList()->format().style() == QTextListFormat::ListDecimal);
	}
	else
	{
		ui->actionBullet_List->setChecked(false);
		ui->actionNumber_List->setChecked(false);
	}
}

void MainWindow::updateCategoryList()
{QMessageBox::critical(this, "Not Implemented", "Need to implement this..."); }

void MainWindow::updateNoteList(QModelIndex index )
{
	notesModel->setFilter("category_id=" + categoriesModel->record(index.row()).value("id").toString());
	notesModel->setSort(2, Qt::AscendingOrder);
	notesModel->select();
	ui->listNotes->selectionModel()->setCurrentIndex(notesModel->index(0, 2), QItemSelectionModel::SelectCurrent);
	updateNoteText(ui->listNotes->currentIndex());
}

void MainWindow::updateNoteText(QModelIndex index )
{
	if(notesModel->rowCount() == 0)
	{
		currentNote = -1;
		ui->textNote->setText("");
	}
	else if(index.row()>-1)
	{
		currentNote = index.row();
		ui->textNote->setText(notesModel->record(currentNote).value("note_text").toString());
	}
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::FocusOut)
	{
		if (object == ui->textNote)
		{
			saveNoteText();
		}
	}
	else if (event->type() == QEvent::KeyPress)
	{
		if(object == ui->textNote)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			// trap tab key
			if (keyEvent->key() == Qt::Key_Tab) {
				// increase indent in lists
				if (ui->textNote->textCursor().currentList())
					increaseIndent();
				// add two spaces in non-lists
				else {
					ui->textNote->insertPlainText("\t");
				}
				return true;
			}
			// trap backtab key
			else if(keyEvent->key() == Qt::Key_Backtab)
			{
				// decrease indent if list
				if (ui->textNote->textCursor().currentList()) {
					decreaseIndent();
				}
				return true;
			}
		}
	}
	return false;
}

void MainWindow::saveNoteText()
{
	QString id = notesModel->record(currentNote).value("id").toString();
	QString text = ui->textNote->document()->toHtml();
	text.replace("'", "''");
	QSqlQuery query(db);
	QString queryString = "update notes set note_text = '" + text + "' where id = " + id;
	query.exec(queryString);
	notesModel->submitAll();
	notesModel->select();
}

//////////////////////////////
// System Tray
//////////////////////////////
void MainWindow::createActions()
 {
	 minimizeAction = new QAction(tr("Mi&nimize"), this);
	 connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

	 maximizeAction = new QAction(tr("Ma&ximize"), this);
	 connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

	 restoreAction = new QAction(tr("&Restore"), this);
	 connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

	 quitAction = new QAction(tr("&Quit"), this);
	 connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
 }

 void MainWindow::createTrayIcon()
 {
	 trayIconMenu = new QMenu(this);
	 trayIconMenu->addAction(minimizeAction);
	 trayIconMenu->addAction(maximizeAction);
	 trayIconMenu->addAction(restoreAction);
	 trayIconMenu->addSeparator();
	 trayIconMenu->addAction(quitAction);

	 trayIcon = new QSystemTrayIcon(this);
	 trayIcon->setContextMenu(trayIconMenu);
	 trayIcon->setIcon(QPixmap(":/icons/brainstorm.png"));
	 trayIcon->show();
 }

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason) { showNormal(); }
void MainWindow::restoreBrainstorm()
{
	this->showNormal();
	this->activateWindow();
	ui->textNote->setFocus();
}
