/*!
 * e2-sat-editor/src/gui/tools.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <filesystem>

#include <QtGlobal>
#include <QTimer>
#include <QRegularExpression>
#include <QDialog>
#include <QGridLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QWidgetAction>
#include <QMouseEvent>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

#include "../e2se_defs.h"

#include "platforms/platform.h"

#include "tools.h"
#include "theme.h"
#include "tab.h"
#include "gui.h"
#include "dialConvertM3u.h"

using namespace e2se;

namespace e2se_gui
{

tools::tools(tab* tid, gui* gid, QWidget* cwid, dataHandler* data)
{
	this->log = new logger("gui", "tools");

	this->gid = gid;
	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->theme = new e2se_gui::theme;
	this->inspect_curr = INSPECT_FILTER::AllLog;
}

tools::~tools()
{
	debug("~tools");

	delete this->theme;
	delete this->log;
}

void tools::inspector()
{
	debug("inspector");

	QDialog* dial = new QDialog(nullptr, Qt::WindowStaysOnTopHint);
	dial->setObjectName("inspector");
	dial->setWindowTitle(tr("Log Inspector", "dialog"));
	dial->setMinimumSize(450, 520);

#ifdef Q_OS_WIN
	theme->win_flavor_fix(dial);
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	dial->connect(dial, &QDialog::finished, [=]() { QTimer::singleShot(0, [=]() { this->inspectReset(); delete dial; }); });
#else
	dial->connect(dial, &QDialog::finished, [=]() { this->inspectReset(); delete dial; });
#endif

	QGridLayout* dfrm = new QGridLayout(dial);

	QTextEdit* dcnt = new QTextEdit;
	dcnt->setReadOnly(true);
	//TODO i18n rtl QTextDocument
	QString textAlign = QApplication::layoutDirection() == Qt::LeftToRight ? "left" : "right";
	dcnt->document()->setDefaultStyleSheet("* { margin: 0; padding: 0 } i { font-style: normal } pre { font-size: 11px; text-align: " + textAlign + " }");
	dcnt->setHtml("</div>");
#ifdef Q_OS_WIN
	if (! theme::isOverridden() && (theme::isFluentWin() || theme::absLuma() || ! theme::isDefault()))
	{
		QStyle* style;

		if (theme::isFluentWin())
			style = QStyleFactory::create("windows11");
		else
			style = QStyleFactory::create("fusion");

		dcnt->verticalScrollBar()->setStyle(style);
		dcnt->horizontalScrollBar()->setStyle(style);
	}
#endif
	platform::osTextEdit(dcnt);

	QComboBox* dtft = new QComboBox;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	dtft->setPlaceholderText(QString("<%1>").arg(tr("Filter", "ui")));
#endif
	dtft->addItem(tr("All Log"));
	dtft->addItem("Debug");
	dtft->addItem("Info");
	dtft->addItem("Error");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtft->connect(dtft, &QComboBox::currentIndexChanged, [=](int index) {
#else
	dtft->connect(dtft, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
#endif
		this->inspectUpdate(dcnt, index);
	});
	platform::osComboBox(dtft);

	inspectUpdate(dcnt);

	dfrm->setContentsMargins(0, 0, 0, 0);
	dfrm->addWidget(dcnt);
	dfrm->addWidget(dtft);
	dial->setLayout(dfrm);
	dial->open();

	QTimer* timer = new QTimer(dial);
	timer->callOnTimeout([=]() { this->inspectUpdate(dcnt, this->inspect_curr); });
	timer->start(1000);
}

//TODO TEST escape html
QString tools::inspectContent(string str, int filter)
{
	QString text = QString::fromStdString(str);
	QString separator;

	if (text.endsWith("\r\n"))
		separator = "\r\n";
	else if (text.endsWith("\r"))
		separator = "\r";
	else
		separator = "\n";

	if (text.endsWith(separator))
		text.remove(text.size() - separator.size(), separator.size());

	QStringList data = text.split(separator);

	QString selector;
	switch (filter)
	{
		case INSPECT_FILTER::Debug: selector = "<Debug>"; break;
		case INSPECT_FILTER::Info: selector = "<Info>"; break;
		case INSPECT_FILTER::Error: selector = "<Error>"; break;
	}

	bool valid = true;

	for (int i = 0; i < data.size(); i++)
	{
		if (filter && ! data[i].contains(selector))
		{
			data[i] = "";
		}
		else
		{
			QString before = data[i];

			if (data[i].replace(QRegularExpression("^([^ ]+ [^ ]+) <([^>]+)> ([^ ]+) ([^:]+)::([^:]+)(.*)$"), "<pre>\\1 <i>&lt;\\2&gt;</i> \\3 <b>\\4</b>::\\5\\6</pre>") == before)
			{
				valid = false;
				break;
			}
		}
	}

	return valid ? data.join("\n") : "";
}

void tools::inspectUpdate(QTextEdit* view, int filter)
{
	if (this->inspect_curr == filter)
	{
		if (this->log->size() != this->inspect_pos)
		{
			view->append(inspectContent(this->log->str().substr(this->inspect_pos), filter));
			this->inspect_pos = this->log->size();
		}
	}
	else
	{
		view->setHtml("</div>");
		view->setHtml(inspectContent(this->log->str(), filter));
		this->inspect_pos = this->log->size();
		this->inspect_curr = static_cast<INSPECT_FILTER>(filter);
	}
}

void tools::inspectReset()
{
	this->inspect_pos = 0;
	this->inspect_curr = INSPECT_FILTER::AllLog;
}

void tools::status(QString message)
{
	if (tid->statusBarIsVisible())
		tid->statusBarMessage(message);
}

void tools::done()
{
	if (tid->statusBarIsVisible())
		tid->statusBarMessage(tr("Done", "message"));
	else
		tid->infoMessage(tr("Done!", "message"));
}

void tools::applyUtils(int bit, e2db::uoopts& opts)
{
	debug("applyUtils", "bit", bit);

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	bool ran = false;

	theme::setWaitCursor();

	try
	{
		ran = true;

		switch (bit)
		{
			case gui::TAB_ATS::UtilsOrphaned_services:
				status(tr("Remove orphaned services", "menu"));
				dbih->remove_orphaned_services();
			break;
			case gui::TAB_ATS::UtilsOrphaned_references:
				status(tr("Remove orphaned references", "menu"));
				dbih->remove_orphaned_references();
			break;
			case gui::TAB_ATS::UtilsFixRemove:
				status(tr("Fix (remove) reference with errors", "menu"));
				dbih->fix_remove_references();
			break;
			case gui::TAB_ATS::UtilsFixDVBNS:
				status(tr("Recalculate DVBNS for services", "menu"));
				dbih->fix_dvbns();
			break;
			case gui::TAB_ATS::UtilsFixBouquets:
				status(tr("Fix bouquets", "menu"));
				dbih->fix_bouquets(false);
			break;
			case gui::TAB_ATS::UtilsFixBouquetsUniq:
				status(tr("Fix bouquets unique userbouquets", "menu"));
				dbih->fix_bouquets(true);
			break;
			case gui::TAB_ATS::UtilsClearServicesCached:
				status(tr("Remove service cached", "menu"));
				dbih->clear_services_cached();
			break;
			case gui::TAB_ATS::UtilsClearServicesCAID:
				status(tr("Remove service CAID", "menu"));
				dbih->clear_services_caid();
			break;
			case gui::TAB_ATS::UtilsClearServicesFlags:
				status(tr("Remove service flags", "menu"));
				dbih->clear_services_flags();
			break;
			case gui::TAB_ATS::UtilsClearServicesData:
				status(tr("Remove all service data", "menu"));
				dbih->clear_services_data();
			break;
			case gui::TAB_ATS::UtilsClearFavourites:
				status(tr("Remove unreferenced entries (favourites)", "menu"));
				dbih->clear_favourites();
			break;
			case gui::TAB_ATS::UtilsClearBouquetsUnused:
				status(tr("Remove from bouquets (unused services)", "menu"));
				dbih->clear_bouquets_unused_services();
			break;
			case gui::TAB_ATS::UtilsRemove_parentallock:
				status(tr("Remove parental lock", "menu"));
				dbih->remove_parentallock();
			break;
			case gui::TAB_ATS::UtilsRemove_bouquets:
				status(tr("Remove all bouquets", "menu"));
				dbih->remove_bouquets();
			break;
			case gui::TAB_ATS::UtilsRemove_userbouquets:
				status(tr("Remove all userbouquets", "menu"));
				dbih->remove_userbouquets();
			break;
			case gui::TAB_ATS::UtilsDuplicates_all:
				status(tr("Remove all duplicates", "menu"));
				dbih->remove_duplicates();
			break;
			case gui::TAB_ATS::UtilsDuplicates_transponders:
				status(tr("Remove duplicate transponders", "menu"));
				dbih->remove_duplicates_transponders();
			break;
			case gui::TAB_ATS::UtilsDuplicates_services:
				status(tr("Remove duplicate services", "menu"));
				dbih->remove_duplicates_services();
			break;
			case gui::TAB_ATS::UtilsDuplicates_references:
				status(tr("Remove duplicate references", "menu"));
				dbih->remove_duplicates_references();
			break;
			case gui::TAB_ATS::UtilsDuplicates_markers:
				status(tr("Remove duplicate markers (names)", "menu"));
				dbih->remove_duplicates_markers();
			break;
			case gui::TAB_ATS::UtilsTransform_tunersets:
				status(tr("Transform transponders to XML settings", "menu"));
				dbih->transform_transponders_to_tunersets();
			break;
			case gui::TAB_ATS::UtilsTransform_transponders:
				status(tr("Transform XML settings to transponders", "menu"));
				dbih->transform_tunersets_to_transponders();
			break;
			case gui::TAB_ATS::UtilsSort_transponders:
				showSortMenu(SORT_ITEM::item_transponder, opts);
				dbih->sort_transponders(opts);
			break;
			case gui::TAB_ATS::UtilsSort_services:
				showSortMenu(SORT_ITEM::item_service, opts);
				dbih->sort_services(opts);
			break;
			case gui::TAB_ATS::UtilsSort_userbouquets:
				showSortMenu(SORT_ITEM::item_userbouquet, opts);
				dbih->sort_userbouquets(opts);
			break;
			case gui::TAB_ATS::UtilsSort_references:
				showSortMenu(SORT_ITEM::item_reference, opts);
				dbih->sort_references(opts);
			break;
			default:
				ran = false;
		}
	}
	catch (...)
	{
		ran = false;

		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(tr("Utils Error", "error"), tr("Error executing utils.", "error")); }, Qt::QueuedConnection);
	}

	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::utilsNotice); }, Qt::QueuedConnection);

	if (ran)
		done();
	else
		return;

	dbih->clearStorage();

	tid->reload();
}

void tools::execMacro(vector<string> pattern)
{
	debug("execMacro");

	auto* dbih = this->data->dbih;

	e2db::uoopts opts;

	this->data->clearErrors();

	bool ran = false;

	theme::setWaitCursor();

	status(tr("Executing macro …", "message"));

	try
	{
		for (string & fn : pattern)
		{
			if (fn.empty())
				ran = false;
			else if (fn == "remove_orphaned_services")
				dbih->remove_orphaned_services();
			else if (fn == "remove_orphaned_references")
				dbih->remove_orphaned_references();
			else if (fn == "fix_remove_references")
				dbih->fix_remove_references();
			else if (fn == "fix_bouquets")
				dbih->fix_bouquets(false);
			else if (fn == "fix_bouquets_uniq")
				dbih->fix_bouquets(true);
			else if (fn == "fix_dvbns")
				dbih->fix_dvbns();
			else if (fn == "clear_services_cached")
				dbih->clear_services_cached();
			else if (fn == "clear_services_caid")
				dbih->clear_services_caid();
			else if (fn == "clear_services_flags")
				dbih->clear_services_flags();
			else if (fn == "clear_services_data")
				dbih->clear_services_data();
			else if (fn == "clear_favourites")
				dbih->clear_favourites();
			else if (fn == "clear_bouquets_unused_services")
				dbih->clear_bouquets_unused_services();
			else if (fn == "remove_parentallock")
				dbih->remove_parentallock();
			else if (fn == "remove_bouquets")
				dbih->remove_bouquets();
			else if (fn == "remove_userbouquets")
				dbih->remove_userbouquets();
			else if (fn == "remove_duplicates")
				dbih->remove_duplicates();
			else if (fn == "remove_duplicates_transponders")
				dbih->remove_duplicates_transponders();
			else if (fn == "remove_duplicates_services")
				dbih->remove_duplicates_services();
			else if (fn == "remove_duplicates_references")
				dbih->remove_duplicates_references();
			else if (fn == "remove_duplicates_markers")
				dbih->remove_duplicates_markers();
			else if (fn == "transform_tunersets_to_transponders")
				dbih->transform_tunersets_to_transponders();
			else if (fn == "transform_transponders_to_tunersets")
				dbih->transform_transponders_to_tunersets();
			else if (fn == "sort_transponders")
				dbih->sort_transponders(opts);
			else if (fn == "sort_services")
				dbih->sort_services(opts);
			else if (fn == "sort_userbouquets")
				dbih->sort_userbouquets(opts);
			else if (fn == "sort_references")
				dbih->sort_references(opts);
			else
				ran = false;
		}

		ran = true;
	}
	catch (...)
	{
		ran = false;

		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(tr("Utils Error", "error"), tr("Error executing macro.", "error")); }, Qt::QueuedConnection);
	}

	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::utilsNotice); }, Qt::QueuedConnection);

	if (ran)
		done();
	else
		return;

	dbih->clearStorage();

	tid->reload();
}

void tools::macroAutofix()
{
	debug("macroAutofix");

	vector<string> pattern = {
		"remove_duplicates",
		"fix_remove_references",
		"fix_dvbns",
		"fix_bouquets"
	};

	execMacro(pattern);
}

void tools::showSortMenu(SORT_ITEM model, e2db::uoopts& opts)
{
	debug("showSortMenu");

	QWidget* wid = tid->lastPopupFocusWidget();
	QPoint pos = tid->lastPopupFocusPos();
	QMenu* menu = sortPopupMenu(model);

	platform::osMenuPopup(menu, wid, pos);

	QMouseEvent mouseRelease(QEvent::MouseButtonRelease, pos, wid->mapToGlobal(QPoint(0, 0)), Qt::LeftButton, Qt::MouseButtons(Qt::LeftButton), {});
	QCoreApplication::sendEvent(wid, &mouseRelease);

	//TODO FIX
	wid->setFocus(Qt::ActiveWindowFocusReason);

	//TODO
	if (this->sort_curr != nullptr)
	{
		opts.prop = this->sort_curr->property("prop").toString().toStdString();
		opts.order = static_cast<e2db::SORT_ORDER>(this->sort_curr->property("order").toInt());
		if (! this->sort_curr->property("selection").toBool())
			opts.selection.clear();
	}
}

QMenu* tools::sortPopupMenu(SORT_ITEM model)
{
	QMenu* menu = new QMenu;
	vector<QWidget*> fields;
	QString title;

	switch (model)
	{
		case SORT_ITEM::item_transponder: title = tr("Sort transponders"); break;
		case SORT_ITEM::item_service: title = tr("Sort services"); break;
		case SORT_ITEM::item_userbouquet: title = tr("Sort userbouquets"); break;
		case SORT_ITEM::item_reference: title = tr("Sort references"); break;
	}

	{
		QWidgetAction* action = new QWidgetAction(nullptr);
		QLabel* label = new QLabel(title);
#ifndef Q_OS_MAC
		label->setStyleSheet("QLabel { margin: 5px 10px }");
#else
		label->setStyleSheet("QLabel { margin: 5px 10px; font-weight: bold }");
#endif
		action->setDefaultWidget(label);
		menu->addAction(action);
	}
	{
		QWidgetAction* action = new QWidgetAction(nullptr);
		QWidget* wrap = new QWidget;
		QFormLayout* form = new QFormLayout(wrap);
		form->setContentsMargins(15, 3, 15, 10);
		form->setFormAlignment(Qt::AlignLeading);
		form->setRowWrapPolicy(QFormLayout::DontWrapRows);

		{
			QComboBox* select = new QComboBox;
			select->setProperty("field", "by");
			fields.emplace_back(select);
			for (auto & x : this->sortComboBoxProps(model))
				select->addItem(x.first, x.second);
			platform::osComboBox(select);
			form->addRow("by", select);
		}
		{
			QComboBox* select = new QComboBox;
			select->setProperty("field", "order");
			fields.emplace_back(select);
			select->addItem(tr("ascending"), e2db::SORT_ORDER::sort_asc);
			select->addItem(tr("descending"), e2db::SORT_ORDER::sort_desc);
			platform::osComboBox(select);
			form->addRow("order", select);
		}

		action->setDefaultWidget(wrap);
		menu->addAction(action);
	}
	if (1)
	{
		menu->addSeparator();
		{
			QWidgetAction* action = new QWidgetAction(nullptr);
			QWidget* wrap = new QWidget;
			QFormLayout* form = new QFormLayout(wrap);
			form->setContentsMargins(15, 8, 15, 8);
			form->setFormAlignment(Qt::AlignLeading);
			form->setVerticalSpacing(12);

			{
				QCheckBox* checker = new QCheckBox;
				checker->setProperty("field", "recall");
				fields.emplace_back(checker);
				checker->setText(tr("Recall this set when sort from context menu"));
				form->addRow(checker);
			}
			{
				form->addItem(form->spacerItem());

				QCheckBox* checker = new QCheckBox;
				checker->setProperty("field", "selection");
				fields.emplace_back(checker);
				checker->setText(tr("Apply to list selection"));
				form->addRow(checker);
			}

			action->setDefaultWidget(wrap);
			menu->addAction(action);
		}
	}
	{
		QWidgetAction* action = new QWidgetAction(nullptr);
		QWidget* wrap = new QWidget;
		QFormLayout* form = new QFormLayout(wrap);
		form->setContentsMargins(15, 10, 15, 10);
		form->setFormAlignment(Qt::AlignTrailing);
		form->setRowWrapPolicy(QFormLayout::DontWrapRows);

		QPushButton* button = new QPushButton;
		button->setDefault(true);
		button->setText(tr("Apply Sort"));
		button->connect(button, &QPushButton::pressed, [=]() {
			this->handleSortOptions(fields);

			//TODO FIX delete menu and aboutToHide platform_macx
		});
		form->addRow(button);

		action->setDefaultWidget(wrap);
		menu->addAction(action);
	}

	return menu;
}

void tools::handleSortOptions(vector<QWidget*> fields)
{
	debug("handleSortOptions");

	this->sort_curr = new QObject;

	for (auto & item : fields)
	{
		QString key = item->property("field").toString();
		QVariant val;

		if (QComboBox* field = qobject_cast<QComboBox*>(item))
			val = field->currentData();
		else if (QCheckBox* field = qobject_cast<QCheckBox*>(item))
			val = field->isChecked();

		if (key == "by")
			this->sort_curr->setProperty("prop", val);
		else if (key == "order")
			this->sort_curr->setProperty("order", val);
		else if (key == "selection")
			this->sort_curr->setProperty("selection", val);
		else if (key == "recall")
			this->sort_curr->setProperty("recall", val);
	}
}

void tools::importFileCSV(e2db::FCONVS fci, e2db::fcopts opts)
{
	debug("importFileCSV");

	vector<string> paths;

	paths = gid->importFileDialog(gui::GUI_DPORTS::CSV);

	if (paths.empty())
		return;

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	if (tid->statusBarIsVisible())
	{
		string fname;
		if (paths.size() > 1)
			fname = std::filesystem::path(paths[0]).parent_path().u8string();
		else
			fname = paths[0];

		tid->statusBarMessage(tr("Importing from %1 …", "message").arg(fname.data()));
	}

	bool read = false;

	theme::setWaitCursor();

	try
	{
		dbih->import_csv_file(fci, opts, paths);

		read = true;
	}
	catch (...)
	{
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(tr("File Error", "error"), tr("Error opening files.", "error")); }, Qt::QueuedConnection);
	}

	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::importNotice); }, Qt::QueuedConnection);

	if (! read)
		return;

	dbih->clearStorage();

	tid->reload();
}

void tools::exportFileCSV(e2db::FCONVS fco, e2db::fcopts opts)
{
	debug("exportFileCSV");

	string path = gid->exportFileDialog(gui::GUI_DPORTS::CSV, opts.filename);

	if (path.empty())
	{
		return;
	}
	if (opts.fc != e2db::FCONVS::convert_current)
	{
		int dirsize = 0;

		try
		{
			string basedir;
			if (std::filesystem::is_directory(path))
				basedir = path;
			else
				basedir = std::filesystem::path(path).parent_path().u8string();

			std::filesystem::directory_iterator dirlist (basedir);

			for (const auto & entry : dirlist)
			{
				if (std::filesystem::is_regular_file(entry))
					dirsize++;
			}
		}
		catch (const std::filesystem::filesystem_error& err)
		{
			error("exportFileCSV", tr("File Error", "error").toStdString(), e2se::logger::msg(e2se::logger::MSG::except_filesystem, err.what()));

			tid->errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

			return;
		}
		if (dirsize != 0)
		{
			bool overwriting = tid->overwriteQuestion();
			if (! overwriting)
				return;
		}
	}

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	bool write = false;

	theme::setWaitCursor();

	try
	{
		dbih->export_csv_file(fco, opts, path);

		write = true;
	}
	catch (...)
	{
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(tr("File Error", "error"), tr("Error writing files.", "error")); }, Qt::QueuedConnection);
	}

	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::exportNotice); }, Qt::QueuedConnection);

	if (! write)
		return;

	if (tid->statusBarIsVisible())
	{
		string filename;
		if (opts.fc != e2db::FCONVS::convert_current)
			filename = std::filesystem::path(path).parent_path().u8string();
		else
			filename = path;

		tid->statusBarMessage(tr("Exported to %1", "message").arg(filename.data()));
	}
	else
	{
		tid->infoMessage(tr("Saved!", "message"));
	}
}

void tools::exportFileHTML(e2db::FCONVS fco, e2db::fcopts opts)
{
	debug("exportFileHTML");

	string path = gid->exportFileDialog(gui::GUI_DPORTS::HTML, opts.filename);

	if (path.empty())
	{
		return;
	}
	if (opts.fc != e2db::FCONVS::convert_current)
	{
		int dirsize = 0;

		try
		{
			string basedir;
			if (std::filesystem::is_directory(path))
				basedir = path;
			else
				basedir = std::filesystem::path(path).parent_path().u8string();

			std::filesystem::directory_iterator dirlist (basedir);

			for (const auto & entry : dirlist)
			{
				if (std::filesystem::is_regular_file(entry))
					dirsize++;
			}
		}
		catch (const std::filesystem::filesystem_error& err)
		{
			error("exportFileHTML", tr("File Error", "error").toStdString(), e2se::logger::msg(e2se::logger::MSG::except_filesystem, err.what()));

			tid->errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

			return;
		}
		if (dirsize != 0)
		{
			bool overwriting = tid->overwriteQuestion();
			if (! overwriting)
				return;
		}
	}

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	bool write = false;

	theme::setWaitCursor();

	try
	{
		dbih->export_html_file(fco, opts, path);

		write = true;
	}
	catch (...)
	{
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(tr("File Error", "error"), tr("Error writing files.", "error")); }, Qt::QueuedConnection);
	}

	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::exportNotice); }, Qt::QueuedConnection);

	if (! write)
		return;

	if (tid->statusBarIsVisible())
	{
		string filename;
		if (opts.fc != e2db::FCONVS::convert_current)
			filename = std::filesystem::path(path).parent_path().u8string();
		else
			filename = path;

		tid->statusBarMessage(tr("Exported to %1", "message").arg(path.data()));
	}
	else
	{
		tid->infoMessage(tr("Saved!", "message"));
	}
}

void tools::importFileM3U(e2db::FCONVS fci, e2db::fcopts opts)
{
	debug("importFileM3U");

	vector<string> paths;

	paths = gid->importFileDialog(gui::GUI_DPORTS::M3U);

	if (paths.empty())
		return;

	dialConvertM3u* dialog = new dialConvertM3u(this->data);
	dialog->setImport(opts);
	dialog->display(cwid);
	opts = dialog->getConverterOptions();
	if (dialog->destroy()) return;

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	if (tid->statusBarIsVisible())
	{
		string fname;
		if (paths.size() > 1)
			fname = std::filesystem::path(paths[0]).parent_path().u8string();
		else
			fname = paths[0];

		tid->statusBarMessage(tr("Importing from %1 …", "message").arg(fname.data()));
	}

	bool read = false;

	theme::setWaitCursor();

	try
	{
		dbih->import_m3u_file(fci, opts, paths);

		read = true;
	}
	catch (...)
	{
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(tr("File Error", "error"), tr("Error opening files.", "error")); }, Qt::QueuedConnection);
	}

	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::importNotice); }, Qt::QueuedConnection);

	if (! read)
		return;

	dbih->clearStorage();

	tid->reload();
}

void tools::exportFileM3U(e2db::FCONVS fco, e2db::fcopts opts)
{
	debug("exportFileM3U");

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	vector<string> ubouquets;

	for (auto & x : dbih->index["ubs"])
	{
		for (auto & q : dbih->userbouquets[x.second].channels)
		{
			if (q.second.stream)
			{
				e2db::userbouquet uboq = dbih->userbouquets[x.second];
				string bname = uboq.bname;
				ubouquets.emplace_back(bname);
				break;
			}
		}
	}

	if (ubouquets.size() == 0)
	{
		tid->infoMessage(tr("Nothing to export", "message"), tr("There are not stream type references in userbouquets to export.", "message"));

		return;
	}
	else if (opts.fc == e2db::FCONVS::convert_current)
	{
		bool found = false;

		for (string & bname : ubouquets)
		{
			if (bname == opts.bname)
			{
				found = true;
				break;
			}
		}

		if (! found)
		{
			opts.fc = e2db::FCONVS::convert_all;
			opts.bname = "";
		}
	}

	dialConvertM3u* dialog = new dialConvertM3u(this->data);
	dialog->setExport(opts, ubouquets);
	dialog->display(cwid);
	opts = dialog->getConverterOptions();
	if (dialog->destroy()) return;

	string path = gid->exportFileDialog(gui::GUI_DPORTS::M3U, opts.filename);

	if (path.empty())
	{
		return;
	}
	if (opts.fc != e2db::FCONVS::convert_current)
	{
		int dirsize = 0;

		try
		{
			string basedir;
			if (std::filesystem::is_directory(path))
				basedir = path;
			else
				basedir = std::filesystem::path(path).parent_path().u8string();

			std::filesystem::directory_iterator dirlist (basedir);

			for (const auto & entry : dirlist)
			{
				if (std::filesystem::is_regular_file(entry))
					dirsize++;
			}
		}
		catch (const std::filesystem::filesystem_error& err)
		{
			error("exportFileM3U", tr("File Error", "error").toStdString(), e2se::logger::msg(e2se::logger::MSG::except_filesystem, err.what()));

			tid->errorMessage(tr("File Error", "error"), tr("Error opening files.", "error"));

			return;
		}
		if (dirsize != 0)
		{
			bool overwriting = tid->overwriteQuestion();
			if (! overwriting)
				return;
		}
	}

	bool write = false;

	theme::setWaitCursor();

	try
	{
		dbih->export_m3u_file(fco, opts, ubouquets, path);

		write = true;
	}
	catch (...)
	{
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(tr("File Error", "error"), tr("Error writing files.", "error")); }, Qt::QueuedConnection);
	}

	theme::unsetWaitCursor();

	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::exportNotice); }, Qt::QueuedConnection);

	if (! write)
		return;

	if (tid->statusBarIsVisible())
	{
		string filename;
		if (opts.fc != e2db::FCONVS::convert_current)
			filename = std::filesystem::path(path).parent_path().u8string();
		else
			filename = path;

		tid->statusBarMessage(tr("Exported to %1", "message").arg(filename.data()));
	}
	else
	{
		tid->infoMessage(tr("Saved!", "message"));
	}
}

vector<QPair<QString, QString>> tools::sortComboBoxProps(SORT_ITEM model)
{
	if (model == SORT_ITEM::item_userbouquet)
	{
		return {
			{tr("Filename"), "ubname"},
			{tr("Bouquet Name"), "name"},
			{tr("Parent Filename"), "pname"},
			{tr("Bouquet Type"), "utype"},
			{tr("Parental Lock"), "parental"},
			{tr("Index"), "index"}
		};
	}
	else if (model == SORT_ITEM::item_reference)
	{
		return {
			{tr("Channel Name"), "chname"},
			{tr("SSID"), "ssid"},
			{tr("TSID"), "tsid"},
			{tr("ONID"), "onid"},
			{tr("DVBNS"), "dvbns"},
			{tr("URI"), "uri"},
			{tr("FAV Name"), "value"},
			{tr("FAV Index"), "inum"},
			{tr("Index"), "index"}
		};
	}
	else if (model == SORT_ITEM::item_service)
	{
		return {
			{tr("Channel Name"), "chname"},
			{tr("Provider Name"), "stype"},
			{tr("SSID"), "ssid"},
			{tr("TSID"), "tsid"},
			{tr("ONID"), "onid"},
			{tr("DVBNS"), "dvbns"},
			{tr("Service Type"), "stype"},
			{tr("Service Number"), "stype"},
			{tr("Src ID"), "stype"},
			{tr("Parental Lock"), "parental"},
			{tr("Index"), "index"}
		};
	}
	else if (model == SORT_ITEM::item_transponder)
	{
		return {
			{tr("TSID"), "tsid"},
			{tr("ONID"), "onid"},
			{tr("DVBNS"), "dvbns"},
			{tr("Transponder Type"), "ytype"},
			{tr("Position"), "pos"},
			{tr("Frequency"), "freq"},
			{tr("SR"), "sr"},
			{tr("Polarization"), "pol"},
			{tr("System"), "sys"},
			{tr("Index"), "index"}
		};
	}

	return {};
}


void tools::destroy()
{
	debug("destroy");

	delete this;
}

}
