/*!
 * e2-sat-editor/src/gui/piconsView.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.7
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QTimer>
#include <QRegularExpression>
#include <QSettings>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QToolBar>
#include <QMenu>
#include <QFileDialog>
#include <QClipboard>
#include <QMimeData>

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

	QVBoxLayout* abox = new QVBoxLayout;

	QGroupBox* afrm = new QGroupBox(tr("Picons"));

	frm->setContentsMargins(0, 0, 0, 0);
	abox->setContentsMargins(0, 0, 0, 0);

	frm->setSpacing(0);
	abox->setSpacing(0);

	afrm->setFlat(true);

	this->tree = new QTreeWidget;
	this->list = new QListWidget;

	list->setStyleSheet("QListWidget { border-style: none } QListWidget::item { min-width: 128px; min-height: 102px; margin: 10px 12px; padding: 12px 8px 2px; border: 8px dashed palette(mid); border-radius: 15px } QListWidget::item:selected { background-color: palette(highlight) }");

	// tree acts as shadow model for list
	tree->setColumnCount(13);

	list->setViewMode(QListWidget::IconMode);
	list->setResizeMode(QListWidget::Adjust);
	list->setSelectionRectVisible(true);
	list->setUniformItemSizes(true);
	list->setIconSize(QSize(112, 60));
	list->setSelectionBehavior(QListWidget::SelectItems);
	list->setSelectionMode(QListWidget::SingleSelection);
	list->setDropIndicatorShown(true);
	list->setDragDropMode(QListWidget::DragDrop);
	// list->setEditTriggers(QListWidget::NoEditTriggers);

	list->setContextMenuPolicy(Qt::CustomContextMenu);
	list->connect(list, &QListWidget::customContextMenuRequested, [=](QPoint pos) { this->showListEditContextMenu(pos); });

	searchLayout();

	QToolBar* list_ats = toolBar();

	browseLayout();

	this->action.list_browse = toolBarWidget(list_ats, list_browse);
	toolBarSpacer(list_ats);
	this->action.list_search = toolBarButton(list_ats, tr("Find…", "toolbar"), theme->dynamicIcon("search"), [=]() { this->listSearchToggle(); });

	this->action.list_search->setDisabled(true);

	this->list_evth = new ListIconDragDropEventHandler(list);
	this->list_evte = new ListIconDragDropEventFilter;
	list->viewport()->installEventFilter(list_evte);
	widget->installEventFilter(list_evth);
	widget->setAcceptDrops(true);

	list->connect(list, &QListWidget::currentItemChanged, [=]() { this->listItemChanged(); });
	list->connect(list, &QListWidget::itemSelectionChanged, [=]() { this->listItemSelectionChanged(); });

	abox->addWidget(list);
	abox->addWidget(list_search);
	abox->addWidget(list_ats);
	afrm->setLayout(abox);

	frm->addWidget(afrm);

	toolBarStyleSheet();
}

void piconsView::browseLayout()
{
	debug("browseLayout");

	this->state.picons_dir = QSettings().value("application/latestPiconsBrowsePath").toString().toStdString();

	this->list_browse = new QWidget;

	QFormLayout* browsef = new QFormLayout;
	browsef->setFormAlignment(Qt::AlignLeading);
	browsef->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout* browsebox = new QHBoxLayout;

	QLineEdit* browseinput = new QLineEdit;
	browseinput->setReadOnly(true);
	browseinput->setText(QString::fromStdString(this->state.picons_dir));

	QPushButton* browsebutton = new QPushButton;
	browsebutton->setText(tr("&Browse…", "toolbar"));
	browsebutton->connect(browsebutton, &QPushButton::pressed, [=]() {
#ifndef E2SE_DEMO
		string curr_dir = this->state.picons_dir;
		string dir = this->browseFileDialog(curr_dir);

		if (curr_dir != dir)
		{
			this->state.picons_dir = dir;
			browseinput->setText(QString::fromStdString(dir));
			QSettings().setValue("application/latestPiconsBrowsePath", QString::fromStdString(dir));

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

	tree->clear();

	list->reset();
	list->setDragEnabled(false);
	// list->setAcceptDrops(false);
	list->clear();

	this->lsr_find.curr = -1;
	this->lsr_find.match.clear();

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
		QString path = filename.prepend("/").prepend(QString::fromStdString(this->state.picons_dir));
		if (! this->state.picons_dir.empty() && QFile::exists(path))
			item->setIcon(QIcon(path));
		else
			item->setIcon(QIcon(":/icons/picon.png"));

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
	// list->setAcceptDrops(true);
}

void piconsView::listItemChanged()
{
	debug("listItemChanged");
}

void piconsView::listItemSelectionChanged()
{
	debug("listItemSelectionChanged");

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
		tabSetFlag(gui::TabListEditService, true);
	}
	else
	{
		tabSetFlag(gui::TabListEditService, false);
	}

	if (QGuiApplication::clipboard()->text().isEmpty())
		tabSetFlag(gui::TabListPaste, false);
	else
		tabSetFlag(gui::TabListPaste, true);

	tabUpdateFlags();
}

//TODO FIX multiple selection with shortcut FindNext when search_box is closed
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
		QModelIndex start = tree->model()->index(0, column);
		int limit = -1;
		match = tree->model()->match(start, Qt::UserRole, text, limit, Qt::MatchFlag::MatchContains);

		if (this->lsr_find.flag == LIST_FIND::all)
			listFindClear();

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
			listFindClear(false);

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

void piconsView::listFindClear(bool hidden)
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
		QStringList qch = item->data(Qt::UserRole).toStringList();
		QString path = qch[ITEM_DATA_ROLE::filename].prepend("/").prepend(QString::fromStdString(this->state.picons_dir));
		if (! this->state.picons_dir.empty() && QFile::exists(path))
			item->setIcon(QIcon(path));
		else
			item->setIcon(QIcon(":/icons/picon.png"));
		i++;
	}
}

void piconsView::updateStatusBar(bool current)
{
	debug("updateStatusBar");
}

void piconsView::showListEditContextMenu(QPoint& pos)
{
	debug("showListEditContextMenu");
}

void piconsView::updateFlags()
{
	debug("updateFlags");

	tabSetFlag(gui::TabTreeEdit, false);
	tabSetFlag(gui::TabTreeDelete, false);
	tabSetFlag(gui::TabTreeFind, false);

	if (list->count())
	{
		tabSetFlag(gui::TabListSelectAll, true);
		tabSetFlag(gui::TabListFind, true);
		this->action.list_search->setEnabled(true);
	}
	else
	{
		tabSetFlag(gui::TabListSelectAll, false);
		tabSetFlag(gui::TabListFind, false);
		this->action.list_search->setDisabled(true);
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

QString piconsView::piconPathname(string chname)
{
	QString qstr = QString::fromStdString(chname).toLower();
	qstr.replace("æ", "ae");
	qstr.replace("œ", "oe");
	qstr.replace("+", "plus");
	qstr = qstr.normalized(QString::NormalizationForm_KD);
	qstr.remove(QRegularExpression("[^a-z0-9]"));

	return qstr;
}

string piconsView::browseFileDialog(string path)
{
#ifndef E2SE_DEMO
	return QFileDialog::getExistingDirectory(nullptr, tr("Select picons folder", "file-dialog"), QString::fromStdString(path)).toStdString();
#else
	return "";
#endif
}

}
