/*!
 * e2-sat-editor/src/gui/inspector.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QtGlobal>
#include <QTimer>
#include <QRegularExpression>
#include <QGridLayout>
#include <QComboBox>
#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QScrollBar>
#endif

#include "../e2se_defs.h"

#include "platforms/platform.h"

#include "inspector.h"
#include "theme.h"
#include "gui.h"

using namespace e2se;

namespace e2se_gui
{

inspector::inspector()
{
	this->log = new logger("gui", "inspector");

	this->theme = new e2se_gui::theme;
	this->inspect_curr = INSPECT_FILTER::AllLog;
}

inspector::~inspector()
{
	debug("~inspector");

	delete this->theme;
	delete this->log;
}

DialogDockWidget* inspector::logInspector()
{
	debug("logInspector");

	if (this->dial == nullptr)
		layout();

	return this->dial;
}

void inspector::layout()
{
	debug("layout");

	this->dial = new DialogDockWidget(nullptr, Qt::WindowStaysOnTopHint);
	dial->setObjectName("logInspector");
	dial->setWindowTitle(tr("Log Inspector", "dialog"));
	dial->setBaseSize(450, 520);
	dial->setMinimumSize(450, 240);
	dial->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

#ifdef Q_OS_WIN
	theme->win_flavor_fix(dial);
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	dial->connect(dial, &DialogDockWidget::finished, [=]() { QTimer::singleShot(0, [=]() { this->close(); }); });
#else
	dial->connect(dial, &DialogDockWidget::finished, [=]() { this->close(); });
#endif

	QGridLayout* dfrm = new QGridLayout;

	QTextEdit* dcnt = new QTextEdit;
	dcnt->setReadOnly(true);
	//TODO i18n rtl QTextDocument
	dcnt->document()->setDefaultStyleSheet("* { margin: 0; padding: 0 } i { font-style: normal } pre { font-size: 11px }");
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

	QTimer* timer = new QTimer(dial);
	timer->callOnTimeout([=]() { this->inspectUpdate(dcnt, this->inspect_curr); });
	timer->start(1000);
}

//TODO TEST escape html
QString inspector::inspectContent(string str, int filter)
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

void inspector::inspectUpdate(QTextEdit* view, int filter)
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

void inspector::inspectReset()
{
	this->inspect_pos = 0;
	this->inspect_curr = INSPECT_FILTER::AllLog;
}

void inspector::close()
{
	if (this->dial == nullptr)
		return;

	debug("close");

	this->dial->deleteLater();
	this->dial = nullptr;
}

void inspector::destroy()
{
	debug("destroy");

	delete this;
}

}
