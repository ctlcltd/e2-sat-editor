/*!
 * e2-sat-editor/src/gui/tools.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.8
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

#include "../e2se_defs.h"

#include "platforms/platform.h"

#include "tools.h"
#include "tab.h"
#include "gui.h"

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
	this->inspect_curr = INSPECT_FILTER::AllLog;
}

tools::~tools()
{
	debug("~tools");

	delete this->log;
}

void tools::inspector()
{
	debug("inspector");

	QDialog* dial = new QDialog(nullptr, Qt::WindowStaysOnTopHint);
	dial->setObjectName("inspector");
	dial->setWindowTitle(tr("Log Inspector", "dialog"));
	dial->setMinimumSize(450, 520);

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
	dtft->connect(dtft, &QComboBox::currentIndexChanged, [=](int index) { this->inspectUpdate(dcnt, index); });
#else
	dtft->connect(dtft, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) { this->inspectUpdate(dcnt, index); });
#endif
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

//TODO html escape cli
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

void tools::importFileCSV(e2db::FCONVS fci, e2db::fcopts opts)
{
	debug("importFileCSV");

	vector<string> paths;

	paths = gid->importFileDialog(gui::GUI_DPORTS::CSV);
	if (paths.empty())
		return;

	auto* dbih = this->data->dbih;
	bool merge = dbih->get_input().size() != 0 ? true : false;

	if (tid->statusBarIsVisible())
	{
		string fname;
		if (paths.size() > 0)
			fname = std::filesystem::path(paths[0]).parent_path().u8string();
		else
			fname = paths[0];

		tid->statusBarMessage(tr("Importing from %1 …", "message").arg(fname.data()));
	}

	theme::setWaitCursor();
	dbih->import_csv_file(fci, opts, paths);
	theme::unsetWaitCursor();

	tid->reset();

	dbih->cache(merge);
	dbih->fixBouquets();

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
		if (dirsize != 0)
		{
			bool overwrite = tid->saveQuestion(tr("The destination contains files that will be overwritten.", "message"), tr("Do you want to overwrite them?", "message"));
			if (! overwrite)
				return;
		}
	}

	auto* dbih = this->data->dbih;

	theme::setWaitCursor();
	dbih->export_csv_file(fco, opts, path);
	theme::unsetWaitCursor();

	if (tid->statusBarIsVisible())
		tid->statusBarMessage(tr("Exported to %1", "message").arg(path.data()));
	else
		tid->infoMessage(tr("Saved!", "message"));
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
		if (dirsize != 0)
		{
			bool overwrite = tid->saveQuestion(tr("The destination contains files that will be overwritten.", "message"), tr("Do you want to overwrite them?", "message"));
			if (! overwrite)
				return;
		}
	}

	auto* dbih = this->data->dbih;

	theme::setWaitCursor();
	dbih->export_html_file(fco, opts, path);
	theme::unsetWaitCursor();

	if (tid->statusBarIsVisible())
		tid->statusBarMessage(tr("Exported to %1", "message").arg(path.data()));
	else
		tid->infoMessage(tr("Saved!", "message"));
}

void tools::destroy()
{
	debug("destroy");

	delete this;
}

}
