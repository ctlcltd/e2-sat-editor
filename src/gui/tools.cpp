/*!
 * e2-sat-editor/src/gui/tools.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.5
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <filesystem>

#include <QtGlobal>
#include <QDialog>
#include <QTimer>
#include <QGridLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>

#include <QMessageBox>

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
}

tools::~tools()
{
	debug("~tools");

	delete this->log;
}

//TODO improve
void tools::inspector()
{
	QDialog* dial = new QDialog(nullptr, Qt::WindowStaysOnTopHint);
	dial->setWindowTitle(tr("Inspector Log"));
	dial->setMinimumSize(450, 520);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	dial->connect(dial, &QDialog::finished, [=]() { QTimer::singleShot(0, [=]() { delete dial; }); });
#else
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; });
#endif

	QGridLayout* dfrm = new QGridLayout(dial);
	QTextEdit* dcnt = new QTextEdit;
	dcnt->setReadOnly(true);
	dcnt->document()->setDefaultStyleSheet("pre { font-size: 11px }");
	dcnt->setHtml("<pre>" + QString::fromStdString(this->log->str()).toHtmlEscaped() + "</pre>");
	platform::osTextEdit(dcnt);

	// QTextEdit* dtdg = new QTextEdit;
	QComboBox* dtft = new QComboBox;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	dtft->setPlaceholderText("<Filter>");
#endif
	dtft->addItem(tr("All Log"));
	dtft->addItem("Debug");
	dtft->addItem("Info");
	dtft->addItem("Error");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtft->connect(dtft, &QComboBox::currentIndexChanged, []() { QMessageBox(QMessageBox::NoIcon, "TODO", "TODO").exec(); });
#else
	dtft->connect(dtft, QOverload<int>::of(&QComboBox::currentIndexChanged), []() { QMessageBox(QMessageBox::NoIcon, "TODO", "TODO").exec(); });
#endif
	platform::osComboBox(dtft);

	dfrm->setContentsMargins(0, 0, 0, 0);
	dfrm->addWidget(dcnt);
	// dfrm->addWidget(dtdg);
	dfrm->addWidget(dtft);
	dial->setLayout(dfrm);
	dial->open();

	QTimer* timer = new QTimer(dial);
	timer->callOnTimeout([=]() {
		if (this->log->pos() != this->log->last_pos())
		{
			dcnt->moveCursor(QTextCursor::End);
			dcnt->append("<pre>" + QString::fromStdString(this->log->str_lend()).toHtmlEscaped() + "</pre>");
			// dtdg->setPlainText(dcnt->toHtml());
		}
	});
	timer->start(1000);
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

		tid->statusBarMessage(tr("Importing from %1 …").arg(QString::fromStdString(fname)));
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
			bool overwrite = tid->saveQuestion(tr("The destination contains files that will be overwritten."), tr("Do you want to overwrite them?"));
			if (! overwrite)
				return;
		}
	}

	auto* dbih = this->data->dbih;

	theme::setWaitCursor();
	dbih->export_csv_file(fco, opts, path);
	theme::unsetWaitCursor();

	if (tid->statusBarIsVisible())
		tid->statusBarMessage(tr("Exported to %1").arg(QString::fromStdString(path)));
	else
		tid->infoMessage(tr("Saved!"));
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
			bool overwrite = tid->saveQuestion(tr("The destination contains files that will be overwritten."), tr("Do you want to overwrite them?"));
			if (! overwrite)
				return;
		}
	}

	auto* dbih = this->data->dbih;

	theme::setWaitCursor();
	dbih->export_html_file(fco, opts, path);
	theme::unsetWaitCursor();

	if (tid->statusBarIsVisible())
		tid->statusBarMessage(tr("Exported to %1").arg(QString::fromStdString(path)));
	else
		tid->infoMessage(tr("Saved!"));
}

void tools::destroy()
{
	debug("destroy");

	delete this;
}

}
