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

#include <QDialog>
#include <QTimer>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>

#include "tools.h"
#include "todo.h"

using namespace e2se;

namespace e2se_gui_tools
{

tools::tools(QGridLayout* root, e2se::logger::session* log)
{
	this->log = new logger(log, "tools");
	debug("tools()");

	this->root = root;
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
	dtft->setPlaceholderText("<Filter>");
	dtft->addItem("All Log");
	dtft->addItem("Debug");
	dtft->addItem("Info");
	dtft->addItem("Error");
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
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

void tools::destroy()
{
	debug("destroy()");
}

}
