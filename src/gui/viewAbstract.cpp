/*!
 * e2-sat-editor/src/gui/viewAbstract.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <cmath>

#include <QtGlobal>
#include <QApplication>
#include <QTimer>
#include <QGridLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QHeaderView>
#include <QStyle>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QToolBar>
#include <QMenu>

#include "platforms/platform.h"

#include "viewAbstract.h"
#include "tab.h"

namespace e2se_gui
{

void viewAbstract::themeChanged()
{
	debug("themeChanged");

	theme->changed();
}

void viewAbstract::searchLayout()
{
	this->tree_search = new QWidget;
	this->list_search = new QWidget;

	tree_search->setObjectName("tree_search");
	list_search->setObjectName("list_search");
	tree_search->setHidden(true);
	list_search->setHidden(true);
	tree_search->setAutoFillBackground(true);
	list_search->setAutoFillBackground(true);

#ifndef Q_OS_MAC
	QColor searchbackground;
	QColor searchcolor;
	QColor searchhighlight = QPalette().color(QPalette::Highlight);
	QString searchbackground_hexArgb;
	QString searchcolor_hexArgb;
	QString searchhighlight_hexArgb;

	searchbackground = QPalette().color(QPalette::Mid).lighter();
	searchcolor = QColor(Qt::black);
	searchhighlight = searchhighlight.lighter(150);
	searchbackground_hexArgb = searchbackground.name(QColor::HexArgb);
	searchcolor_hexArgb = searchcolor.name(QColor::HexArgb);
	searchhighlight_hexArgb = searchhighlight.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "#tree_search, #list_search { color: " + searchcolor_hexArgb + "; background: " + searchbackground_hexArgb + " } #list_search_highlight { background: " + searchbackground_hexArgb + " } #list_search_highlight:checked { background: " + searchhighlight_hexArgb + " }", theme::light);

	searchbackground = QPalette().color(QPalette::Mid).darker();
	searchcolor = QColor(Qt::white);
	searchhighlight = searchhighlight.darker(200);
	searchbackground_hexArgb = searchbackground.name(QColor::HexArgb);
	searchcolor_hexArgb = searchcolor.name(QColor::HexArgb);
	searchhighlight_hexArgb = searchhighlight.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "#tree_search, #list_search { color: " + searchcolor_hexArgb + "; background: " + searchbackground_hexArgb + " } #list_search_highlight { background: " + searchbackground_hexArgb + " } #list_search_highlight:checked { background: " + searchhighlight_hexArgb + " }", theme::dark);
#else
	QColor searchbackground;
	QColor searchhighlight = QPalette().color(QPalette::Highlight);
	QString searchbackground_hexArgb;
	QString searchhighlight_hexArgb;

	searchbackground = QColor(Qt::white).darker(102);
	searchhighlight = searchhighlight.lighter(107);
	searchbackground_hexArgb = searchbackground.name(QColor::HexArgb);
	searchhighlight_hexArgb = searchhighlight.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "#tree_search, #list_search { background: " + searchbackground_hexArgb + " } #list_search_highlight { background: " + searchbackground_hexArgb + " } #list_search_highlight:checked { background: " + searchhighlight_hexArgb + " }", theme::light);

	searchbackground = QPalette().color(QPalette::Mid).darker(227);
	searchhighlight = searchhighlight.darker(122);
	searchbackground_hexArgb = searchbackground.name(QColor::HexArgb);
	searchhighlight_hexArgb = searchhighlight.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "#tree_search, #list_search { background: " + searchbackground_hexArgb + " } #list_search_highlight { background: " + searchbackground_hexArgb + " } #list_search_highlight:checked { background: " + searchhighlight_hexArgb + " }", theme::dark);
#endif

	platform::osWidgetOpaque(tree_search);
	platform::osWidgetOpaque(list_search);

	QGridLayout* tsr_box = new QGridLayout(tree_search);
	tsr_box->setContentsMargins(4, 3, 3, 6);
	tsr_box->setSpacing(0);
	this->tsr_search.input = new QLineEdit;
	this->tsr_search.input->connect(this->tsr_search.input, &QLineEdit::textChanged, [=](const QString& text) { this->treeFindPerform(text); });
	platform::osLineEdit(this->tsr_search.input);

	this->tsr_search.next = new QPushButton("Find");
	this->tsr_search.next->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->tsr_search.next->connect(this->tsr_search.next, &QPushButton::pressed, [=]() { this->treeFindPerform(); });

	this->tsr_search.close = new QPushButton;
	this->tsr_search.close->setIconSize(QSize(10, 10));
	this->tsr_search.close->setIcon(theme->dynamicIcon("close", this->tsr_search.close));
	this->tsr_search.close->setFlat(true);
	this->tsr_search.close->setMaximumWidth(22);
	this->tsr_search.close->connect(this->tsr_search.close, &QPushButton::pressed, [=]() { this->treeSearchClose(); });

	tsr_box->addItem(new QSpacerItem(5, 0), 0, 0);
	tsr_box->addWidget(this->tsr_search.input, 0, 1);
	tsr_box->addItem(new QSpacerItem(2, 0), 0, 2);
	tsr_box->addWidget(this->tsr_search.next, 0, 3);
	tsr_box->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 4);
	tsr_box->addWidget(this->tsr_search.close, 0, 5);

	QGridLayout* lsr_box = new QGridLayout(list_search);
	lsr_box->setContentsMargins(4, 3, 3, 6);
	lsr_box->setSpacing(0);
	this->lsr_search.filter = new QComboBox;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	this->lsr_search.filter->connect(this->lsr_search.filter, &QComboBox::currentIndexChanged, [=]() { this->listFindReset(); });
#else
	this->lsr_search.filter->connect(this->lsr_search.filter, QOverload<int>::of(&QComboBox::currentIndexChanged), [=]() { this->listFindReset(); });
#endif
	platform::osComboBox(this->lsr_search.filter);

	this->lsr_search.input = new QLineEdit;
	this->lsr_search.input->setMinimumWidth(180);
	this->lsr_search.input->setStyleSheet("QLineEdit { padding: 2px 0 }");
	this->lsr_search.input->connect(this->lsr_search.input, &QLineEdit::textChanged, [=](const QString& text) { this->listFindPerform(text, LIST_FIND::fast); });
	platform::osLineEdit(this->lsr_search.input);

	this->lsr_search.highlight = new QPushButton;
	this->lsr_search.highlight->setObjectName("list_search_highlight");
	this->lsr_search.highlight->setText("Highlight");
	this->lsr_search.highlight->setCheckable(true);
	this->lsr_search.highlight->setChecked(true);
	this->lsr_search.highlight->setStyleSheet("QPushButton, QPushButton:checked { margin: 0 2px; padding: 2px 2ex; border: 1px solid palette(button); border-radius: 2px; color: palette(highlight-text) }");
	this->lsr_search.highlight->connect(this->lsr_search.highlight, &QPushButton::pressed, [=]() { this->listFindHighlightToggle(); });

	this->lsr_search.next = new QPushButton("Find");
	this->lsr_search.next->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->lsr_search.next->connect(this->lsr_search.next, &QPushButton::pressed, [=]() { this->listFindPerform(LIST_FIND::next); });

	this->lsr_search.prev = new QPushButton("Find Previous");
	this->lsr_search.prev->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->lsr_search.prev->connect(this->lsr_search.prev, &QPushButton::pressed, [=]() { this->listFindPerform(LIST_FIND::prev); });

	this->lsr_search.all = new QPushButton("Find All");
	this->lsr_search.all->setStyleSheet("QPushButton, QPushButton:pressed { margin: 0 2px; padding: 3px 2ex; border: 1px solid transparent; border-radius: 3px; background: palette(button) } QPushButton:pressed { background: palette(light) }");
	this->lsr_search.all->connect(this->lsr_search.all, &QPushButton::pressed, [=]() { this->listFindPerform(LIST_FIND::all); });

	this->lsr_search.close = new QPushButton;
	this->lsr_search.close->setIconSize(QSize(10, 10));
	this->lsr_search.close->setIcon(theme->dynamicIcon("close", this->lsr_search.close));
	this->lsr_search.close->setFlat(true);
	this->lsr_search.close->setMaximumWidth(28);
	this->lsr_search.close->connect(this->lsr_search.close, &QPushButton::pressed, [=]() { this->listSearchClose(); });

	lsr_box->addWidget(this->lsr_search.filter, 0, 0);
	lsr_box->addWidget(this->lsr_search.input, 0, 1);
	lsr_box->addItem(new QSpacerItem(2, 0), 0, 2);
	lsr_box->addWidget(this->lsr_search.next, 0, 3);
	lsr_box->addWidget(this->lsr_search.prev, 0, 4);
	lsr_box->addWidget(this->lsr_search.all, 0, 5);
	lsr_box->addItem(new QSpacerItem(16, 0), 0, 6);
	lsr_box->addWidget(this->lsr_search.highlight, 0, 7);
	lsr_box->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 8);
	lsr_box->addWidget(this->lsr_search.close, 0, 9);
}

void viewAbstract::sortByColumn(int column)
{
	debug("sortByColumn", "column", column);

	Qt::SortOrder order = list->header()->sortIndicatorOrder();
	column = column == 1 ? 0 : column;

	// sorting by
	if (column)
	{
		list->sortItems(column, order);
		list->header()->setSortIndicatorShown(true);
	}
	// sorting default
	else
	{
		list->sortItems(column, order);
		list->header()->setSortIndicator(1, order);

		// default column 0|asc
		if (order == Qt::AscendingOrder)
			list->header()->setSortIndicatorShown(false);
		else
			list->header()->setSortIndicatorShown(true);
	}
}

void viewAbstract::treeItemDelete()
{
	debug("treeItemDelete");

	QList<QTreeWidgetItem*> selected = tree->selectedItems();

	if (selected.empty())
	{
		return;
	}
	for (auto & item : selected)
	{
		QTreeWidgetItem* parent = item->parent();
		parent->removeChild(item);
	}
}

void viewAbstract::listItemCut()
{
	debug("listItemCut");

	listItemCopy(true);
}

void viewAbstract::listItemSelectAll()
{
	debug("listItemSelectAll");

	list->selectAll();
}

void viewAbstract::treeSearchShow()
{
	debug("treeSearchShow");

	tree_search->show();
}

void viewAbstract::treeSearchHide()
{
	debug("treeSearchHide");

	tree_search->hide();
}

void viewAbstract::treeSearchToggle()
{
	// debug("treeSearchToggle");

	if (tree_search->isHidden())
	{
		treeSearchShow();
	}
	else
	{
		treeSearchHide();
	}
}

void viewAbstract::treeSearchClose()
{
	// debug("treeSearchClose");

	QTimer::singleShot(100, [=]() {
		treeSearchHide();
	});
}

void viewAbstract::listSearchShow()
{
	debug("listSearchShow");

	list_search->show();
	this->lsr_find.timer.start();
}

void viewAbstract::listSearchHide()
{
	debug("listSearchHide");

	list_search->hide();
	if (! this->lsr_find.highlight)
		listFindClear();
}

void viewAbstract::listSearchToggle()
{
	// debug("listSearchToggle");

	if (list_search->isHidden())
		listSearchShow();
	else
		listSearchHide();
}

void viewAbstract::listSearchClose()
{
	// debug("listSearchClose");

	QTimer::singleShot(100, [=]() {
		listSearchHide();
	});
}

void viewAbstract::treeFindPerform()
{
	if (this->tsr_search.input->text().isEmpty())
		return;

	treeFindPerform(this->tsr_search.input->text());
}

void viewAbstract::treeFindPerform(const QString& value)
{
	tree->keyboardSearch(value);

	tabSetFlag(gui::TabTreeFind, true);
	if (tree->currentIndex().isValid())
		tabSetFlag(gui::TabTreeFindNext, true);
	else
		tabSetFlag(gui::TabTreeFindNext, false);
}

void viewAbstract::listFindPerform(LIST_FIND flag)
{
	if (this->lsr_search.input->text().isEmpty())
		return;

	listFindPerform(this->lsr_search.input->text(), flag);
}

//TODO FIX multiple selection with shortcut FindNext when search_box is closed
void viewAbstract::listFindPerform(const QString& value, LIST_FIND flag)
{
	// QTreeWidgetItem* item list->currentItem() || list->topLevelItem(0)
	// int column || QTreeWidget::currentColumn()
	// void QTreeWidget::setCurrentItem(QTreeWidgetItem* item, int column)
	// void QTreeView::keyboardSearch(const QString& search)

	int column = this->lsr_search.filter->currentData().toInt();
	int delay = flag == LIST_FIND::fast ? QApplication::keyboardInputInterval() : 0;
	QString text;

	if (flag == LIST_FIND::fast)
	{
		bool keyboardTimeWasValid = this->lsr_find.timer.isValid();
		qint64 elapsed;
		if (keyboardTimeWasValid)
			elapsed = this->lsr_find.timer.restart();
		else
			this->lsr_find.timer.start();
		if (value.isEmpty() || ! keyboardTimeWasValid || elapsed > delay)
		{
			text = value;

			listFindClear();
		}
		else
		{
			text += value;
		}
	}
	else
	{
		if (value.isEmpty())
			return;
		else
			text = value;
	}

	QModelIndexList match;

	if (this->lsr_find.match.isEmpty() || this->lsr_find.filter != column || this->lsr_find.input != text)
	{
		// fast 0 --> start
		// fast i match(..., ..., ..., 1, ...)
		//
		// QModelIndex start;
		// if (list_tree->currentIndex().isValid())
		//	start = list_tree->currentIndex();
		// else
		// 	start = list_tree->model()->index(0, 0);
		//
		QModelIndex start = list->model()->index(0, column);
		int limit = -1;
		match = list->model()->match(start, Qt::DisplayRole, text, limit, Qt::MatchFlag::MatchContains);

		if (this->lsr_find.flag == LIST_FIND::all)
			listFindClear();

		this->lsr_find.curr = -1;
	}
	else
	{
		match = this->lsr_find.match;

		//TODO improve
		/*if (flag == LIST_FIND::fast && this->lsr_find.flag == LIST_FIND::all)
		{
			if (list->currentIndex().isValid())
				return;
			else
				this->lsr_find.curr = -1;
		}*/
	}

	if (match.count())
	{
		LIST_FIND type = flag == LIST_FIND::fast ? LIST_FIND::next : flag;
		int i = 0;
		int j = int (match.size());

		if (type == LIST_FIND::next)
		{
			i = int (this->lsr_find.curr);
			i = i == j - 1 ? 0 : i + 1;
			list->setCurrentIndex(match.at(i));
		}
		else if (type == LIST_FIND::prev)
		{
			i = int (this->lsr_find.curr);
			i = i <= 0 ? j - 1 : i - 1;
			list->setCurrentIndex(match.at(i));
		}
		else if (type == LIST_FIND::all)
		{

			listFindClear(false);
			while (i != j)
			{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
				QTreeWidgetItem* item = list->itemFromIndex(match.at(i));
#else
				QTreeWidgetItem* item;
				QModelIndex index = match.at(i);
				if (index.isValid())
					item = static_cast<QTreeWidgetItem*>(index.internalPointer());
#endif

				if (this->lsr_find.highlight)
					item->setSelected(true);
				else
					item->setHidden(false);
				i++;
			}
			if (this->lsr_find.highlight)
				list->scrollTo(match.at(0));
			i = -1;
		}
		// int i = list_tree->indexOfTopLevelItem(QTreeWidgetItem* item);
		this->lsr_find.curr = i;

		tabSetFlag(gui::TabListFindNext, true);
		tabSetFlag(gui::TabListFindPrev, true);
		tabSetFlag(gui::TabListFindAll, true);
	}
	else
	{
		tabSetFlag(gui::TabListFindNext, false);
		tabSetFlag(gui::TabListFindPrev, false);
		tabSetFlag(gui::TabListFindAll, false);
	}

	this->lsr_find.flag = flag;
	this->lsr_find.filter = column;
	this->lsr_find.input = text;
	this->lsr_find.match = match;
}

void viewAbstract::listFindHighlightToggle()
{
	listFindReset();
	this->lsr_find.highlight = ! this->lsr_find.highlight;
}

void viewAbstract::listFindClear(bool hidden)
{
	list->clearSelection();

	if (! this->lsr_find.highlight)
	{
		int j = list->topLevelItemCount();
		while (j--)
		{
			QTreeWidgetItem* item = list->topLevelItem(j);
			item->setHidden(! hidden);
		}
	}
}

void viewAbstract::listFindReset()
{
	listFindClear();

	this->lsr_find.flag = LIST_FIND::fast;
	this->lsr_find.filter = 0;
	this->lsr_find.highlight = true;
	this->lsr_find.curr = -1;
	this->lsr_find.input = "";
	this->lsr_find.match.clear();
	this->lsr_find.timer.invalidate();
}

void viewAbstract::tabSetFlag(gui::GUI_CXE bit, bool flag)
{
	if (tid != nullptr)
		tid->setFlag(bit, flag);
}

void viewAbstract::tabSetFlag(gui::GUI_CXE bit)
{
	if (tid != nullptr)
		tid->setFlag(bit);
}

bool viewAbstract::tabGetFlag(gui::GUI_CXE bit)
{
	if (tid != nullptr)
		return tid->getFlag(bit);
	return true;
}

void viewAbstract::tabUpdateFlags()
{
	if (tid != nullptr)
		tid->storeFlags();
}

void viewAbstract::tabUpdateFlags(gui::GUI_CXE bit)
{
	tabSetFlag(bit);
	tabUpdateFlags();
}

void viewAbstract::tabUpdateToolBars()
{
	if (tid != nullptr)
		tid->updateToolBars();
}

bool viewAbstract::tabRemoveQuestion(QString title, QString text)
{
	if (tid != nullptr)
		return tid->removeQuestion(title, text);
	return false;
}

bool viewAbstract::tabStatusBarIsVisible()
{
	if (tid != nullptr)
		return tid->statusBarIsVisible();
	return false;
}

bool viewAbstract::tabStatusBarIsHidden()
{
	if (tid != nullptr)
		return tid->statusBarIsHidden();
	return true;
}

void viewAbstract::tabSetStatusBar(gui::status msg)
{
	if (tid != nullptr)
		tid->setStatusBar(msg);
}

void viewAbstract::tabResetStatusBar()
{
	if (tid != nullptr)
		tid->resetStatusBar();
}

void viewAbstract::tabNewFile()
{
	if (tid != nullptr)
		tid->newFile();
}

void viewAbstract::tabOpenFile()
{
	if (tid != nullptr)
		tid->openFile();
}

bool viewAbstract::tabReadFile(string filename)
{
	if (tid != nullptr)
		return tid->readFile(filename);
	return false;
}

void viewAbstract::tabSaveFile(bool saveas)
{
	if (tid != nullptr)
		tid->saveFile(saveas);
}

void viewAbstract::tabImportFile()
{
	if (tid != nullptr)
		tid->importFile();
}

void viewAbstract::tabExportFile()
{
	if (tid != nullptr)
		tid->exportFile();
}

void viewAbstract::tabPrintFile(bool all)
{
	if (tid != nullptr)
		tid->printFile(all);
}

//TODO viewport minimumWidth QPushButton are not supported
QToolBar* viewAbstract::toolBar()
{
	QToolBar* toolbar = new QToolBar;
	toolbar->setObjectName("view_toolbar");
	toolbar->setIconSize(QSize(12, 12));
	toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	toolbar->setStyleSheet("QToolBar { padding: 0 8px } QToolButton { font: bold 14px }");

	platform::osWidgetOpaque(toolbar);

	return toolbar;
}

QAction* viewAbstract::toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* viewAbstract::toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setIcon(icon.first->dynamicIcon(icon.second, action));
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* viewAbstract::toolBarAction(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QAction* viewAbstract::toolBarAction(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(toolbar);
	action->setText(text);
	action->setIcon(icon.first->dynamicIcon(icon.second, action));
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	toolbar->addAction(action);
	return action;
}

QPushButton* viewAbstract::toolBarButton(QToolBar* toolbar, QString text, std::function<void()> trigger)
{
	QPushButton* button = new QPushButton(toolbar);
	button->setText(text);
	button->connect(button, &QPushButton::pressed, trigger);
	toolbar->addWidget(button);
	return button;
}

QPushButton* viewAbstract::toolBarButton(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger)
{
	QPushButton* button = new QPushButton(toolbar);
	button->setText(text);
	button->setIcon(icon.first->dynamicIcon(icon.second, button));
	button->connect(button, &QPushButton::pressed, trigger);
	toolbar->addWidget(button);
	return button;
}

QPushButton* viewAbstract::toolBarButton(QToolBar* toolbar, QString text, std::function<void()> trigger, QKeySequence shortcut)
{
	QPushButton* button = new QPushButton(toolbar);
	button->setText(text);
	button->setShortcut(shortcut);
	button->connect(button, &QPushButton::pressed, trigger);
	toolbar->addWidget(button);
	return button;
}

QPushButton* viewAbstract::toolBarButton(QToolBar* toolbar, QString text, pair<e2se_gui::theme*, QString> icon, std::function<void()> trigger, QKeySequence shortcut)
{
	QPushButton* button = new QPushButton(toolbar);
	button->setText(text);
	button->setIcon(icon.first->dynamicIcon(icon.second, button));
	button->setShortcut(shortcut);
	button->connect(button, &QPushButton::pressed, trigger);
	toolbar->addWidget(button);
	return button;
}

QWidget* viewAbstract::toolBarWidget(QToolBar* toolbar, QWidget* widget)
{
	toolbar->addWidget(widget);
	return widget;
}

QAction* viewAbstract::toolBarSeparator(QToolBar* toolbar)
{
	QAction* action = new QAction(toolbar);
	action->setSeparator(true);
	toolbar->addAction(action);
	return action;
}

QWidget* viewAbstract::toolBarSpacer(QToolBar* toolbar)
{
	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
	toolbar->addWidget(spacer);
	return spacer;
}

void viewAbstract::toolBarStyleSheet()
{
#ifndef Q_OS_MAC
	if (! theme::isDefault())
	{
		theme->dynamicStyleSheet(widget, "#view_toolbar { background: palette(highlighted-text) }", theme::light);
		theme->dynamicStyleSheet(widget, "#view_toolbar { background: palette(dark) }", theme::dark);
	}
#else
	theme->dynamicStyleSheet(widget, "QToolBar { border: 0; border-top: 1px solid } QToolBar::separator { border: 0 }");

	QColor tbshade;
	QString tbshade_hexArgb;

	tbshade = QColor(Qt::black);
	tbshade.setAlphaF(0.06);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "#view_toolbar { background: rgba(255, 255, 255, 0.33); border-color: " + tbshade_hexArgb + " }", theme::light);

	tbshade = QPalette().color(QPalette::Dark).darker();
	tbshade.setAlphaF(0.27);
	tbshade_hexArgb = tbshade.name(QColor::HexArgb);

	theme->dynamicStyleSheet(widget, "#view_toolbar { background: rgba(0, 0, 0, 0.33); border-color: " + tbshade_hexArgb + " }", theme::dark);
#endif
}

QMenu* viewAbstract::contextMenu()
{
	return new QMenu;
}

QMenu* viewAbstract::contextMenu(QMenu* menu)
{
	return new QMenu(menu);
}

QAction* viewAbstract::contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* viewAbstract::contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, bool enabled)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->setEnabled(enabled);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* viewAbstract::contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, QKeySequence shortcut)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->setShortcut(shortcut);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* viewAbstract::contextMenuAction(QMenu* menu, QString text, std::function<void()> trigger, bool enabled, QKeySequence shortcut)
{
	QAction* action = new QAction(menu);
	action->setText(text);
	action->setShortcut(shortcut);
	action->setEnabled(enabled);
	action->connect(action, &QAction::triggered, trigger);
	menu->addAction(action);
	return action;
}

QAction* viewAbstract::contextMenuSeparator(QMenu* menu)
{
	QAction* action = new QAction(menu);
	action->setSeparator(true);
	menu->addAction(action);
	return action;
}

}
