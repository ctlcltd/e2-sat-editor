/*!
 * e2-sat-editor/src/gui/piconsView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.4.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QTimer>
#include <QRegularExpression>
#include <QSettings>
#include <QMessageBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QMenu>
#include <QWidgetAction>
#include <QClipboard>
#include <QMimeData>
#include <QMouseEvent>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

#include "../e2se_defs.h"
#include "platforms/platform.h"

#include "piconsView.h"
#include "theme.h"
#include "tab.h"
#include "gui.h"

using namespace e2se;

namespace e2se_gui
{

piconsView::piconsView(tab* tid, QWidget* cwid, dataHandler* data)
{
	this->log = new logger("gui", "piconsView");

	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->theme = new e2se_gui::theme;
	this->widget = new QWidget;

	layout();
}

piconsView::~piconsView()
{
	debug("~piconsView");

	delete this->widget;
	delete this->theme;
	delete this->log;
}

void piconsView::layout()
{
	debug("layout");

	widget->setStyleSheet("QGroupBox { spacing: 0; border: 0; padding: 0; padding-top: 32px; font-weight: bold } QGroupBox::title { margin: 8px 4px; padding: 0 1px 1px }");

	QGridLayout* frm = new QGridLayout(widget);

	QGridLayout* awrap = new QGridLayout;
	QVBoxLayout* abox = new QVBoxLayout;

	QGroupBox* afrm = new QGroupBox(tr("Picons"));

	frm->setContentsMargins(0, 0, 0, 0);
	awrap->setContentsMargins(0, 0, 0, 0);
	abox->setContentsMargins(0, 0, 0, 0);

	frm->setSpacing(0);
	awrap->setSpacing(0);
	abox->setSpacing(0);

	afrm->setFlat(true);

	this->tree = new QTreeWidget;
	this->list = new QListWidget;

#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::isFluentWin() || theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		list->verticalScrollBar()->setStyle(style);
		list->horizontalScrollBar()->setStyle(style);
	}
#endif

	list->setStyleSheet("QListWidget { border-style: none } QListWidget::item { min-width: 128px; min-height: 102px; margin: 10px 12px; padding: 12px 8px 2px; border: 8px dashed palette(mid); border-radius: 15px } QListWidget::item:selected { background-color: palette(highlight) }");

	// tree acts as shadow model for list
	tree->setColumnCount(13);

	list->setViewMode(QListWidget::IconMode);
	list->setResizeMode(QListWidget::Adjust);
	list->setSelectionRectVisible(true);
	list->setUniformItemSizes(true);
	list->setIconSize(QSize(112, 60));
	list->setSelectionBehavior(QListWidget::SelectItems);
	list->setSelectionMode(QListWidget::ExtendedSelection);
	list->setDropIndicatorShown(true);
	list->setDragDropMode(QListWidget::DragDrop);
	list->setEditTriggers(QListWidget::NoEditTriggers);

	list->setContextMenuPolicy(Qt::CustomContextMenu);
	list->connect(list, &QListWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	searchLayout();
	browseLayout();

	QWidget* acrn = new QWidget;
	acrn->setStyleSheet("position: absolute; top: 0");
	QHBoxLayout* acrn_box = new QHBoxLayout;
	acrn_box->setContentsMargins(0, 0, 0, 0);
	acrn_box->setSizeConstraint(QLayout::SetMinimumSize);

	this->action.acrn_prefs = new QPushButton;
#ifdef Q_OS_MAC
	this->action.acrn_prefs->setFlat(true);
#endif
	this->action.acrn_prefs->setIcon(theme->dynamicIcon("list-prefs-menu", this->action.acrn_prefs));
	this->action.acrn_prefs->setWhatsThis(tr("Picons Preferences", "corner"));
	this->action.acrn_prefs->setToolTip(tr("Picons Preferences", "corner"));
	this->action.acrn_prefs->connect(this->action.acrn_prefs, &QPushButton::pressed, [=]() {
		QMenu* menu = this->listPrefsCornerMenu();
		// menu->popup(this->action.acrn_prefs->mapToGlobal(this->action.acrn_prefs->pos()));
		platform::osMenuPopup(menu, this->action.acrn_prefs, this->action.acrn_prefs->pos());

		QMouseEvent mouseRelease(QEvent::MouseButtonRelease, this->action.acrn_prefs->pos(), this->action.acrn_prefs->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::MouseButtons(Qt::LeftButton), {});
		QCoreApplication::sendEvent(this->action.acrn_prefs, &mouseRelease);
	});

	acrn_box->addWidget(this->action.acrn_prefs, 0, Qt::AlignTrailing);
	acrn->setLayout(acrn_box);

	QToolBar* list_ats = toolBar();

	this->action.list_browse = toolBarWidget(list_ats, tr("&Browse…", "toolbar"), list_browse);
	toolBarSpacer(list_ats);
	this->action.list_search = toolBarButton(list_ats, tr("Find…", "toolbar"), theme->dynamicIcon("search"), [=]() { this->listSearchToggle(); });

	this->action.list_search->setDisabled(true);
	this->action.list_search->actions().first()->setDisabled(true);

	this->list_evth = new ListIconDragDropEventHandler(list);
	this->list_evte = new ListIconDragDropEventFilter;
	list_evth->setEventCallback([=](QListWidgetItem* item, QString path) { this->changePicon(item, path); });
	list->viewport()->installEventFilter(list_evte);
	widget->installEventFilter(list_evth);
	widget->setAcceptDrops(true);

	list->connect(list, &QListWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list->connect(list, &QListWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });
	list->connect(list, &QListWidget::itemDoubleClicked, [=]() { this->listItemDoubleClicked(); });

	abox->addWidget(list);
	abox->addWidget(list_search);
	abox->addWidget(list_ats);
	afrm->setLayout(abox);
	awrap->addWidget(afrm, 0, 0);
	awrap->addWidget(acrn, 0, 0, Qt::AlignTop | Qt::AlignTrailing);

	QWidget* wrap = new QWidget;
	wrap->setLayout(awrap);
	frm->addWidget(wrap);

	toolBarStyleSheet();
}

void piconsView::browseLayout()
{
	debug("browseLayout");

	this->state.picons_dir = QSettings().value("application/piconsBrowsePath").toString();

	this->list_browse = new QWidget;

	QFormLayout* browsef = new QFormLayout;
	browsef->setFormAlignment(Qt::AlignLeading);
	browsef->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* browsebox = new QHBoxLayout;

	QLineEdit* browseinput = new QLineEdit;
	browseinput->setReadOnly(true);
	browseinput->setText(this->state.picons_dir);
	platform::osLineEdit(browseinput);

	QPushButton* browsebutton = new QPushButton;
	browsebutton->setText(tr("&Browse…", "toolbar"));
	browsebutton->connect(browsebutton, &QPushButton::pressed, [=]() {
#ifndef E2SE_DEMO
		QString curr_dir = this->state.picons_dir;
		QString dir = this->browseFileDialog(curr_dir);

		if (curr_dir != dir)
		{
			this->state.picons_dir = dir;
			browseinput->setText(dir);
			QSettings().setValue("application/piconsBrowsePath", dir);

			listChangedPiconsPath();
		}
#else
		tid->demoMessage();
#endif
	});

	browsebox->addWidget(browseinput);
	browsebox->addWidget(browsebutton);
	browsef->addRow(tr("Folder", "toolbar"), browsebox);

	this->list_browse->setLayout(browsef);
}

void piconsView::searchLayout()
{
	this->viewAbstract::searchLayout();

	this->lsr_search.filter->addItem(tr("Name"), ITEM_DATA_ROLE::chname);
	this->lsr_search.filter->addItem(tr("Type"), ITEM_DATA_ROLE::chtype);
	this->lsr_search.filter->addItem(tr("CAS"), ITEM_DATA_ROLE::chcas);
	this->lsr_search.filter->addItem(tr("Provider"), ITEM_DATA_ROLE::chpname);
	this->lsr_search.filter->addItem(tr("Filename"), ITEM_DATA_ROLE::filename);
	this->lsr_search.filter->addItem(tr("Tuner"), ITEM_DATA_ROLE::txtname);
	this->lsr_search.filter->addItem(tr("Position"), ITEM_DATA_ROLE::txpos);
	this->lsr_search.filter->addItem(tr("System"), ITEM_DATA_ROLE::txsys);
}

void piconsView::load()
{
	debug("load");

	tabUpdateFlags(gui::init);

	populate();

	updateFlags();
	updateStatusBar();
}

void piconsView::reset()
{
	debug("reset");

	this->state.picons_dir = "";
	this->state.curr_picon = "";
	this->state.tab_pending = false;
	this->state.q_piconsUseChname = QSettings().value("preference/piconsUseChname").toBool();

	tree->clear();

	list->reset();
	list->setDragEnabled(false);
	list->setAcceptDrops(false);
	list->clear();

	listFindClear();

	resetStatusBar();
}

void piconsView::populate()
{
	auto* dbih = this->data->dbih;

	tree->clear();
	list->clear();

	QList<QTreeWidgetItem*> s_items;

	if (! dbih->index.count("chs"))
		error("populate", "current", "Missing index key \"chs\".");

	for (auto & chi : dbih->index["chs"])
	{
		QString idx;
		QString chid = QString::fromStdString(chi.second);
		QString chname;
		QString filename;
		QStringList entry;

		if (dbih->db.services.count(chi.second))
		{
			e2db::service ch = dbih->db.services[chi.second];
			entry = dbih->entries.services[chi.second];
			idx = QString::number(chi.first);
			entry.prepend(idx);
			chname = entry[ITEM_DATA_ROLE::chname];

			if (QSettings().value("preference/piconsUseChname").toBool())
			{
				filename = piconPathname(ch.chname);
				if (filename.isEmpty())
				{
					QString refid = QString::fromStdString(dbih->get_reference_id(chi.second));
					filename = QString(refid).replace(":", "_").append(".png");
				}
				else
				{
					filename.append(".png");
				}
			}
			else
			{
				QString refid = QString::fromStdString(dbih->get_reference_id(chi.second));
				filename = QString(refid).replace(":", "_").append(".png");
			}

			entry.insert(ITEM_DATA_ROLE::filename, filename);
		}
		else
		{
			error("populate", tr("Error", "error").toStdString(), tr("Channel mismatch \"%1\".", "error").arg(chi.second.data()).toStdString());
		}

		QTreeWidgetItem* s_item = new QTreeWidgetItem;
		QListWidgetItem* item = new QListWidgetItem;
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemNeverHasChildren);
		item->setText(chname);
		item->setToolTip(filename);

		QString filepath = filename.prepend("/").prepend(this->state.picons_dir);

		item->setData(Qt::UserRole, filepath);

#ifndef Q_OS_WASM
		if (! this->state.picons_dir.isEmpty() && QFile::exists(filepath))
			item->setIcon(QIcon(filepath));
		else
			item->setIcon(QIcon(":/icons/picon.png"));
#endif

		s_item->setData(ITEM_DATA_ROLE::x, Qt::UserRole, entry[ITEM_DATA_ROLE::x]);
		s_item->setData(ITEM_DATA_ROLE::chid, Qt::UserRole, entry[ITEM_DATA_ROLE::chid]);
		s_item->setData(ITEM_DATA_ROLE::txid, Qt::UserRole, entry[ITEM_DATA_ROLE::txid]);
		s_item->setData(ITEM_DATA_ROLE::chname, Qt::UserRole, entry[ITEM_DATA_ROLE::chname]);
		s_item->setData(ITEM_DATA_ROLE::chtype, Qt::UserRole, entry[ITEM_DATA_ROLE::chtype]);
		s_item->setData(ITEM_DATA_ROLE::chcas, Qt::UserRole, entry[ITEM_DATA_ROLE::chcas]);
		s_item->setData(ITEM_DATA_ROLE::chpname, Qt::UserRole, entry[ITEM_DATA_ROLE::chpname]);
		s_item->setData(ITEM_DATA_ROLE::filename, Qt::UserRole, entry[ITEM_DATA_ROLE::filename]);
		s_item->setData(ITEM_DATA_ROLE::txtname, Qt::UserRole, entry[ITEM_DATA_ROLE::txtname]);
		s_item->setData(ITEM_DATA_ROLE::txpos, Qt::UserRole, entry[ITEM_DATA_ROLE::txpos]);
		s_item->setData(ITEM_DATA_ROLE::txsys, Qt::UserRole, entry[ITEM_DATA_ROLE::txsys]);

		list->addItem(item);
		s_items.append(s_item);
	}

	tree->addTopLevelItems(s_items);

	list->setDragEnabled(true);
	list->setAcceptDrops(true);
}

void piconsView::listItemChanged()
{
	// debug("listItemChanged");
}

void piconsView::listItemSelectionChanged()
{
	// debug("listItemSelectionChanged");

	QList<QListWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
	{
		tabSetFlag(gui::TabListCut, false);
		tabSetFlag(gui::TabListCopy, false);
		tabSetFlag(gui::TabListDelete, false);
	}
	else
	{
		tabSetFlag(gui::TabListCut, true);
		tabSetFlag(gui::TabListCopy, true);
		tabSetFlag(gui::TabListDelete, true);
	}

	if (selected.count() == 1)
	{
		tabSetFlag(gui::TabListEditPicon, true);

		QListWidgetItem* item = selected.first();
		this->state.curr_picon = item->toolTip().toStdString();
	}
	else
	{
		tabSetFlag(gui::TabListEditPicon, false);
	}

	if (QGuiApplication::clipboard()->text().isEmpty())
		tabSetFlag(gui::TabListPaste, false);
	else
		tabSetFlag(gui::TabListPaste, true);

	updateStatusBar(selected.count() == 1);

	tabUpdateFlags();
}

void piconsView::listItemDoubleClicked()
{
	debug("listItemDoubleClicked");

	QList<QListWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

	editPicon();
}

//TODO TEST multiple selection with shortcut FindNext when search_box is closed
void piconsView::listFindPerform(const QString& value, LIST_FIND flag)
{
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

			listFindClearSelection();
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
		QModelIndex start = tree->model()->index(0, column);
		int limit = -1;
		match = tree->model()->match(start, Qt::UserRole, text, limit, Qt::MatchFlag::MatchContains);

		if (this->lsr_find.flag == LIST_FIND::all)
			listFindClearSelection();

		this->lsr_find.curr = -1;
	}
	else
	{
		match = this->lsr_find.match;
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
			QModelIndex index = list->model()->index(match.at(i).row(), 0);
			list->setCurrentIndex(index);
		}
		else if (type == LIST_FIND::prev)
		{
			i = int (this->lsr_find.curr);
			i = i <= 0 ? j - 1 : i - 1;
			QModelIndex index = list->model()->index(match.at(i).row(), 0);
			list->setCurrentIndex(index);
		}
		else if (type == LIST_FIND::all)
		{
			listFindClearSelection(false);

			while (i != j)
			{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
				QModelIndex index = list->model()->index(match.at(i).row(), 0);
				QListWidgetItem* item = list->itemFromIndex(index); //Qt5 protected member
#else
				QListWidgetItem* item;
				QModelIndex index = list->model()->index(match.at(i).row(), 0);
				if (index.isValid())
					item = static_cast<QListWidgetItem*>(index.internalPointer());
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

void piconsView::listFindClearSelection(bool hidden)
{
	list->clearSelection();

	if (! this->lsr_find.highlight)
	{
		int j = list->count();
		while (j--)
		{
			QListWidgetItem* item = list->item(j);
			item->setHidden(! hidden);
		}
	}
}

void piconsView::listChangedPiconsPath()
{
	debug("listChangedPiconsPath");

	int i = 0;
	int j = list->count();

	while (i != j)
	{
		QListWidgetItem* item = list->item(i);
		QTreeWidgetItem* s_item = tree->topLevelItem(list->row(item));
		QString filename = s_item->data(ITEM_DATA_ROLE::filename, Qt::UserRole).toString();
		QString filepath = filename.prepend("/").prepend(this->state.picons_dir);

		item->setData(Qt::UserRole, filepath);

		if (! this->state.picons_dir.isEmpty() && QFile::exists(filepath))
			item->setIcon(QIcon(filepath));
		else
			item->setIcon(QIcon(":/icons/picon.png"));

		i++;
	}
}

QMenu* piconsView::listPrefsCornerMenu()
{
	if (this->action.acrn_prefs_menu == nullptr)
	{
		QMenu* menu = this->action.acrn_prefs_menu = new QMenu;

		{
			QWidgetAction* action = new QWidgetAction(nullptr);
			QLabel* label = new QLabel(tr("Picons"));
#ifndef Q_OS_MAC
			label->setStyleSheet("QLabel { margin: 5px 10px }");
#else
			label->setStyleSheet("QLabel { margin: 5px 10px; font-weight: bold }");
#endif
			action->setDefaultWidget(label);
			menu->addAction(action);
		}
		{
			QAction* action = new QAction;
			action->setText(tr("Backup picon when replaced"));
			action->setCheckable(true);
			action->connect(action, &QAction::triggered, [=](bool checked)
			{
				QSettings settings;
				settings.setValue("preference/piconsBackup", checked);
				settings.sync();
				action->setChecked(checked);
			});
			menu->addAction(action);
		}
		{
			menu->addSeparator();
			QWidgetAction* action = new QWidgetAction(nullptr);
			QLabel* label = new QLabel(tr("Filename format"));
#ifndef Q_OS_MAC
			label->setStyleSheet("QLabel { margin: 5px 10px }");
#else
			label->setStyleSheet("QLabel { margin: 5px 10px; font-weight: bold }");
#endif
			action->setDefaultWidget(label);
			menu->addAction(action);
		}
		{
			QAction* action = new QAction;
			action->setText(tr("Use reference ID"));
			action->setCheckable(true);
			action->connect(action, &QAction::triggered, [=](bool checked)
			{
				QSettings settings;
				settings.setValue("preference/piconsUseRefid", checked);
				settings.setValue("preference/piconsUseChname", ! checked);
				settings.sync();
				action->setChecked(checked);

				this->didChange();
			});
			menu->addAction(action);
		}
		{
			QAction* action = new QAction;
			action->setText(tr("Use service name"));
			action->setCheckable(true);
			action->connect(action, &QAction::triggered, [=](bool checked)
			{
				QSettings settings;
				settings.setValue("preference/piconsUseRefid", ! checked);
				settings.setValue("preference/piconsUseChname", checked);
				settings.sync();
				action->setChecked(checked);

				this->didChange();
			});
			menu->addAction(action);
		}
	}

	QMenu* menu = this->action.acrn_prefs_menu;

	auto actions = menu->actions();
	QSettings settings;

	for (auto & act : actions)
		act->setChecked(false);

	actions.at(1)->setChecked(settings.value("preference/piconsBackup", true).toBool());
	actions.at(4)->setChecked(settings.value("preference/piconsUseRefid", true).toBool());
	actions.at(5)->setChecked(settings.value("preference/piconsUseChname", false).toBool());

	return menu;
}

void piconsView::editPicon()
{
	debug("editPicon");

	QList<QListWidgetItem*> selected = list->selectedItems();

	if (selected.empty() || selected.count() > 1)
		return;

#ifdef E2SE_DEMO
	return tid->demoMessage();
#endif

	QListWidgetItem* item = selected.first();

	QString path = this->importFileDialog(this->state.picons_dir);

	changePicon(item, path);
}

void piconsView::changePicon(QListWidgetItem* item, QString path)
{
	debug("changePicon", "index", list->row(item));
	debug("changePicon", "path", path.toStdString());

#ifdef E2SE_DEMO
	return;
#endif

	if (! QFile::exists(path))
	{
		error("changePicon", tr("File Error", "error").toStdString(), tr("Error reading file \"%1\".", "error").arg(path.data()).toStdString());

		return;
	}

	QString filepath = item->data(Qt::UserRole).toString();

	if (QFile::exists(filepath))
	{
		if (QSettings().value("preference/piconsBackup", true).toBool())
		{
			QFileInfo fi = QFileInfo(filepath);
			QString bak = fi.baseName();
			bak.prepend("_bak__").append('.').append(fi.completeSuffix());
			bak = fi.dir().filePath(bak);

			QFile::rename(filepath, bak);
		}

		QFile::remove(filepath);
	}

	QFile::copy(path, filepath);

	if (QFile::exists(filepath))
	{
		item->setIcon(QIcon(filepath));
	}
	else
	{
		error("changePicon", tr("File Error", "error").toStdString(), tr("Error writing file \"%1\".", "error").arg(filepath).toStdString());

		item->setIcon(QIcon(":/icons/picon.png"));
	}
}

//TODO png image
/*void piconsView::changePicon(QListWidgetItem* item, QByteArray data)
{
	debug("changePicon", "index", list->row(item));
	debug("changePicon", "data", true);

	if (data.isEmpty())
	{
		error("changePicon", tr("Error", "error").toStdString(), tr("Not a valid data format.", "error").toStdString());

		return;
	}

	QString filepath = item->data(Qt::UserRole).toString();

	if (QFile::exists(filepath))
	{
		if (QSettings().value("preference/piconsBackup", true).toBool())
		{
			QFileInfo fi = QFileInfo(filepath);
			QString bak = fi.baseName();
			bak.prepend("_bak__").append('.').append(fi.completeSuffix());
			bak = fi.dir().filePath(bak);

			QFile::rename(filepath, bak);
		}

		QFile::remove(filepath);
	}

	QFile file (filepath);
	file.write(data);
	file.close();

	if (QFile::exists(filepath))
	{
		item->setIcon(QIcon(filepath));
	}
	else
	{
		error("changePicon", tr("File Error", "error").toStdString(), tr("Error writing file \"%1\".", "error").arg(filepath).toStdString());

		item->setIcon(QIcon(":/icons/picon.png"));
	}
}*/

//TODO listItemCut
void piconsView::listItemCopy(bool cut)
{
	debug("listItemCopy");

	QList<QListWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

#ifdef E2SE_DEMO
	return;
#endif

	QClipboard* clipboard = QGuiApplication::clipboard();

	/*if (selected.count() > 1)*/
	{
		QMimeData* content = new QMimeData;
		QList<QUrl> urls;

		for (auto & item : selected)
		{
			QString filepath = item->data(Qt::UserRole).toString();

			if (QFile::exists(filepath))
				urls.append(QUrl(filepath));
		}

		content->setUrls(urls);
		clipboard->setMimeData(content);
	}
	/*else
	{
		QListWidgetItem* item = selected.first();
		QString filepath = item->data(Qt::UserRole).toString();

		if (QFile::exists(filepath))
			clipboard->setImage(QImage(filepath));
	}*/

	tabSetFlag(gui::TabListPaste, true);
}

void piconsView::listItemPaste()
{
	debug("listItemPaste");

	QList<QListWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

#ifdef E2SE_DEMO
	return;
#endif

	QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData->hasUrls())
	{
		int i = 0;
		for (auto & item : selected)
		{
			QUrl url = mimeData->urls().at(i);

			if (QFile::exists(url.toLocalFile()))
				changePicon(item, url.toLocalFile());
			if (i++ == mimeData->urls().size())
				break;
		}
	}
	/*else if (mimeData->hasImage() && selected.count() == 1)
	{
		QListWidgetItem* item = selected.first();
		QString filepath = item->data(Qt::UserRole).toString();

		changePicon(item, mimeData->imageData().toByteArray());
	}*/
}

//TODO improve ui
//TODO improve option remember setting
void piconsView::listItemDelete(bool cut)
{
	debug("listItemDelete");

	QList<QListWidgetItem*> selected = list->selectedItems();

	if (selected.empty())
		return;

	QString title = tr("Delete images", "message");
	QString text = tr("Do you want to delete the images from disk?", "message");

	text = text.toHtmlEscaped();
	text.prepend("<span style=\"white-space: nowrap\">");
	text.append("</span><br>");

	QMessageBox msg = QMessageBox(this->cwid);

	msg.setWindowFlags(Qt::Sheet | Qt::MSWindowsFixedSizeDialogHint);
#ifdef Q_OS_MAC
	msg.setAttribute(Qt::WA_TranslucentBackground);
#endif

	msg.setTextFormat(Qt::PlainText);
	msg.setText(title);
	msg.setInformativeText(text);
	msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Retry);
	msg.setDefaultButton(QMessageBox::Ok);

	if (msg.exec() != QMessageBox::Ok)
		return;

#ifdef E2SE_DEMO
	return tid->demoMessage();
#endif

	for (auto & item : selected)
	{
		QTreeWidgetItem* s_item = tree->topLevelItem(list->row(item));
		QString filename = s_item->data(ITEM_DATA_ROLE::filename, Qt::UserRole).toString();
		QString filepath = filename.prepend("/").prepend(this->state.picons_dir);

		if (QFile::exists(filepath))
		{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
			if (! QFile::moveToTrash(filepath))
				QFile::remove(filepath);
#else
			QFile::remove(filepath);
#endif
		}
	}
}

void piconsView::updateStatusBar(bool current)
{
	debug("updateStatusBar");

	gui::status msg;
	msg.update = current;

	if (current && ! this->state.curr_picon.empty())
		msg.curr = this->state.curr_picon;

	tabSetStatusBar(msg);
}

void piconsView::showListEditContextMenu(QPoint& pos)
{
	// debug("showListEditContextMenu");

	QList<QListWidgetItem*> selected = list->selectedItems();

	if (selected.empty() && list->count() != 0)
		return;

	bool editable = false;

	if (selected.count() == 1)
	{
		editable = true;
	}

	QMenu* list_edit = contextMenu();

	contextMenuAction(list_edit, tr("Change picon", "context-menu"), [=]() { this->editPicon(); }, editable && tabGetFlag(gui::TabListEditPicon));
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("Cu&t", "context-menu"), [=]() { this->listItemCut(); }, tabGetFlag(gui::TabListCut), QKeySequence::Cut);
	contextMenuAction(list_edit, tr("&Copy", "context-menu"), [=]() { this->listItemCopy(); }, tabGetFlag(gui::TabListCopy), QKeySequence::Copy);
	contextMenuAction(list_edit, tr("&Paste", "context-menu"), [=]() { this->listItemPaste(); }, tabGetFlag(gui::TabListPaste), QKeySequence::Paste);
	contextMenuSeparator(list_edit);
	contextMenuAction(list_edit, tr("&Delete", "context-menu"), [=]() { this->listItemDelete(); }, tabGetFlag(gui::TabListDelete), QKeySequence::Delete);

	platform::osMenuPopup(list_edit, list, pos);
}

void piconsView::actionCall(int bit)
{
	// debug("actionCall", "bit", bit);

	switch (bit)
	{
		case gui::TAB_ATS::ListEditPicon:
			editPicon();
		break;
	}
}

void piconsView::updateFlags()
{
	debug("updateFlags");

	tabSetFlag(gui::FileImport, false);
	tabSetFlag(gui::FileExport, false);
	tabUpdateToolBars();

	if (list->count())
	{
		tabSetFlag(gui::TabListSelectAll, true);
		tabSetFlag(gui::TabListFind, true);
		this->action.list_search->setEnabled(true);
		this->action.list_search->actions().first()->setEnabled(true);
	}
	else
	{
		tabSetFlag(gui::TabListSelectAll, false);
		tabSetFlag(gui::TabListFind, false);
		this->action.list_search->setDisabled(true);
		this->action.list_search->actions().first()->setDisabled(true);
	}

	if (QGuiApplication::clipboard()->text().isEmpty())
		tabSetFlag(gui::TabListPaste, false);
	else
		tabSetFlag(gui::TabListPaste, true);

	auto* dbih = this->data->dbih;

	if (dbih->index.count("chs"))
	{
		tabSetFlag(gui::Picons, true);
		tabSetFlag(gui::OpenChannelBook, true);
	}
	else
	{
		tabSetFlag(gui::Picons, false);
		tabSetFlag(gui::OpenChannelBook, false);
	}

	tabSetFlag(gui::TabTreeFindNext, false);
	tabSetFlag(gui::TabListFindNext, false);
	tabSetFlag(gui::TabListFindPrev, false);
	tabSetFlag(gui::TabListFindAll, false);

	tabSetFlag(gui::TunersetsSat, true);
	tabSetFlag(gui::TunersetsTerrestrial, true);
	tabSetFlag(gui::TunersetsCable, true);
	tabSetFlag(gui::TunersetsAtsc, true);

	tabUpdateFlags();
}

void piconsView::didChange()
{
	debug("didChange");

	if (this->state.q_piconsUseChname != QSettings().value("preference/piconsUseChname", false).toBool())
	{
		this->state.q_piconsUseChname = QSettings().value("preference/piconsUseChname", false).toBool();

		populate();
	}
}

void piconsView::update()
{
	debug("update");

	if (this->state.tab_pending)
	{
		populate();

		listFindClear();

		this->state.tab_pending = false;
	}
}

void piconsView::updateFromTab()
{
	debug("updateFromTab");

	this->state.tab_pending = true;
}

QString piconsView::piconPathname(string chname)
{
	QString qstr = QString::fromStdString(chname).toLower();
	qstr.replace("+", "plus");
	qstr.replace("&", "and");
	qstr.replace("æ", "ae");
	qstr.replace("œ", "oe");
	qstr = qstr.normalized(QString::NormalizationForm_KD);
	qstr.remove(QRegularExpression("[^a-z0-9]"));

	return qstr;
}

QString piconsView::browseFileDialog(QString path)
{
#ifndef E2SE_DEMO
	return QFileDialog::getExistingDirectory(nullptr, tr("Select picons folder", "file-dialog"), path);
#else
	return "";
#endif
}

QString piconsView::importFileDialog(QString path)
{
#ifndef E2SE_DEMO
	return QFileDialog::getOpenFileName(nullptr, tr("Select picon image", "file-dialog"), path, "Image PNG (*.png)");
#else
	return "";
#endif
}

}
