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
#include <QLineEdit>
#include <QComboBox>
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
#include "convertM3u.h"

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

void tools::applyUtils(int bit, e2db::fcopts opts)
{
	debug("applyUtils", "bit", bit);

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	bool ndial = true;
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
				dbih->removeBouquets();
			break;
			case gui::TAB_ATS::UtilsRemove_userbouquets:
				status(tr("Remove all userbouquets", "menu"));
				dbih->removeUserbouquets();
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
				ndial = true;
				dbih->sort_transponders();
			break;
			case gui::TAB_ATS::UtilsSort_services:
				ndial = true;
				dbih->sort_services();
			break;
			case gui::TAB_ATS::UtilsSort_userbouquets:
				ndial = true;
				dbih->sort_userbouquets();
			break;
			case gui::TAB_ATS::UtilsSort_references:
				ndial = true;
				if (opts.fc == e2db::FCONVS::convert_current)
					dbih->sort_references(opts.bname);
				else
					dbih->sort_references(false);
			break;
			default:
				ran = false;
		}

		if (ndial)
			debug("applyUtils", "TODO", "tools sort dialogs");
	}
	catch (...)
	{
		ran = false;

		//TODO error handling
	}

	theme::unsetWaitCursor();

	//TODO MSG_CODE
	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::importNotice); }, Qt::QueuedConnection);

	if (ran)
		done();
	else
		return;

	tid->reset();

	dbih->clearStorage();

	tid->load();
}

void tools::execMacro(vector<string> methods)
{
	debug("execMacro");

	auto* dbih = this->data->dbih;

	this->data->clearErrors();

	int curr = 0;
	bool ran = false;

	theme::setWaitCursor();

	status(tr("Executing macro …", "message"));

	try
	{
		for (string & method : methods)
		{
			if (method.empty())
				ran = false;
			else if (method == "remove_orphaned_services")
				dbih->remove_orphaned_services();
			else if (method == "remove_orphaned_references")
				dbih->remove_orphaned_references();
			else if (method == "fix_remove_references")
				dbih->fix_remove_references();
			else if (method == "fix_dvbns")
				dbih->fix_dvbns();
			else if (method == "clear_services_cached")
				dbih->clear_services_cached();
			else if (method == "clear_services_caid")
				dbih->clear_services_caid();
			else if (method == "clear_services_flags")
				dbih->clear_services_flags();
			else if (method == "clear_services_data")
				dbih->clear_services_data();
			else if (method == "clear_favourites")
				dbih->clear_favourites();
			else if (method == "clear_bouquets_unused_services")
				dbih->clear_bouquets_unused_services();
			else if (method == "remove_parentallock")
				dbih->remove_parentallock();
			else if (method == "remove_bouquets")
				dbih->removeBouquets();
			else if (method == "remove_userbouquets")
				dbih->removeUserbouquets();
			else if (method == "remove_duplicates")
				dbih->remove_duplicates();
			else if (method == "remove_duplicates_transponders")
				dbih->remove_duplicates_transponders();
			else if (method == "remove_duplicates_services")
				dbih->remove_duplicates_services();
			else if (method == "remove_duplicates_references")
				dbih->remove_duplicates_references();
			else if (method == "remove_duplicates_markers")
				dbih->remove_duplicates_markers();
			else if (method == "transform_tunersets_to_transponders")
				dbih->transform_tunersets_to_transponders();
			else if (method == "transform_transponders_to_tunersets")
				dbih->transform_transponders_to_tunersets();
			else if (method == "sort_transponders")
				dbih->sort_transponders();
			else if (method == "sort_services")
				dbih->sort_services();
			else if (method == "sort_userbouquets")
				dbih->sort_userbouquets();
			else if (method == "sort_references")
				dbih->sort_references(false);
			else
				ran = false;
		}

		ran = true;
	}
	catch (...)
	{
		ran = false;

		//TODO error handling
	}

	theme::unsetWaitCursor();

	//TODO MSG_CODE
	if (this->data->haveErrors())
		QMetaObject::invokeMethod(this->cwid, [=]() { tid->e2dbError(this->data->getErrors(), tab::MSG_CODE::importNotice); }, Qt::QueuedConnection);

	if (ran)
		done();
	else
		return;

	tid->reset();

	dbih->clearStorage();

	tid->load();
}

void tools::macroAutofix()
{
	debug("macroAutofix");

	vector<string> methods = {
		"remove_duplicates",
		"fix_remove_references",
		"fix_dvbns"
	};

	execMacro(methods);
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

	tid->reset();

	dbih->clearStorage();

	tid->load();
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

	convertM3u* dialog = new convertM3u(this->data);
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

	tid->reset();

	dbih->clearStorage();

	tid->load();
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

	convertM3u* dialog = new convertM3u(this->data);
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

void tools::destroy()
{
	debug("destroy");

	delete this;
}

}
