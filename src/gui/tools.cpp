/*!
 * e2-sat-editor/src/gui/tools.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QDialog>
#include <QTimer>
#include <QGridLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>

#include "tools.h"
#include "tab.h"
#include "gui.h"
#include "todo.h"

using namespace e2se;

namespace e2se_gui
{

tools::tools(tab* tid, gui* gid, QWidget* cwid, dataHandler* data, e2se::logger::session* log)
{
	this->log = new logger(log, "tools");
	debug("tools()");

	this->gid = gid;
	this->tid = tid;
	this->cwid = cwid;
	this->data = data;
	this->sets = new QSettings;
}

//TODO improve
void tools::inspector()
{
	QDialog* dial = new QDialog(nullptr, Qt::WindowStaysOnTopHint);
	dial->setWindowTitle("Inspector Log");
	dial->setMinimumSize(450, 520);
	dial->connect(dial, &QDialog::finished, [=]() { delete dial; });

	QGridLayout* dfrm = new QGridLayout(dial);
	QTextEdit* dcnt = new QTextEdit;
	dcnt->setReadOnly(true);
	dcnt->document()->setDefaultStyleSheet("pre { font-size: 11px }");
	dcnt->setHtml("<pre>" + QString::fromStdString(this->log->str()).toHtmlEscaped() + "</pre>");

	// QTextEdit* dtdg = new QTextEdit;
	QComboBox* dtft = new QComboBox;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	dtft->setPlaceholderText("<Filter>");
#endif
	dtft->addItem("All Log");
	dtft->addItem("Debug");
	dtft->addItem("Info");
	dtft->addItem("Error");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	dtft->connect(dtft, &QComboBox::currentIndexChanged, e2se_gui::todo);
#else
	dtft->connect(dtft, QOverload<int>::of(&QComboBox::currentIndexChanged), e2se_gui::todo);
#endif

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
	debug("importFileCSV()");

	vector<string> paths;

	paths = gid->importFileDialog(gui::GUI_DPORTS::CSV);
	if (paths.empty())
		return;

	auto* dbih = this->data->dbih;
	bool merge = dbih->get_input().size() != 0 ? true : false;

	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	dbih->import_csv_file(fci, opts, paths);
	QGuiApplication::restoreOverrideCursor();
	
	tid->reset();

	if (merge)
	{
		dbih->entries.transponders.clear();
		dbih->entries.services.clear();
	}

	for (auto & txdata : dbih->db.transponders)
	{
		dbih->entries.transponders[txdata.first] = dbih->entryTransponder(txdata.second);
	}
	for (auto & chdata : dbih->db.services)
	{
		dbih->entries.services[chdata.first] = dbih->entryService(chdata.second);
	}

	tid->load();

	if (sets->value("application/parserDebugger", false).toBool())
		dbih->debugger();
}

void tools::exportFileCSV(e2db::FCONVS fco, e2db::fcopts opts)
{
	debug("exportFileCSV()");

	string path = gid->exportFileDialog(gui::GUI_DPORTS::CSV, opts.filename);

	if (path.empty())
		return;

	auto* dbih = this->data->dbih;

	//TODO improve ui remove QMessageBox
	QMessageBox msg = QMessageBox();
	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	dbih->export_csv_file(fco, opts, path);
	QGuiApplication::restoreOverrideCursor();
	msg.setText("Saved!");
	msg.exec();
}

void tools::exportFileHTML(e2db::FCONVS fco, e2db::fcopts opts)
{
	debug("exportFileHTML()");

	string path = gid->exportFileDialog(gui::GUI_DPORTS::HTML, opts.filename);

	if (path.empty())
		return;

	auto* dbih = this->data->dbih;

	//TODO improve ui remove QMessageBox
	QMessageBox msg = QMessageBox();
	QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	dbih->export_html_file(fco, opts, path);
	QGuiApplication::restoreOverrideCursor();
	msg.setText("Saved!");
	msg.exec();
}

void tools::destroy()
{
	debug("destroy()");

	delete this;
}

}
