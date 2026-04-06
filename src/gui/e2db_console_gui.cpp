/*!
 * e2-sat-editor/src/gui/e2db_console_gui.cpp
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <cstdio>
#include <ctime>
#include <clocale>
#include <sstream>

#include <QGridLayout>
#include <QTextStream>
#include <QTextDocument>
#include <QTextBlock>
#include <QBuffer>

#include "toolkit/DialogDockWidget.h"
#include "e2db_console_gui.h"

namespace e2se_gui
{

//TODO
e2db_console_gui::e2db_console_gui(QWidget* parent, dataHandler* data)
{
	std::setlocale(LC_NUMERIC, "C");

	this->log = new e2se::logger("gui", "e2db_console_gui");
	this->plog = this->log;

	this->data = data;

	if (__objio.out == OBJIO::byline)
		__objio.hrn = false;
	else if (__objio.out == OBJIO::json)
		__objio.hrn = false;

	this->termctl = new e2se_gui::termctl;

	layout(parent);
	init();
}

void e2db_console_gui::layout(QWidget* parent)
{
	debug("layout");

	QGridLayout* frm = new QGridLayout;
	this->cnt = new ConsoleWidget;

	cnt->setCursorWidth(7);
	cnt->setLineWrapMode(QPlainTextEdit::NoWrap);
	cnt->setWordWrapMode(QTextOption::NoWrap);
	cnt->setAcceptDrops(false);
	cnt->setUndoRedoEnabled(false);
	cnt->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	cnt->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// reflects qtbase/src/gui/text/qtexthtmlparser.cpp for <pre>
	QFont font = QFont (QFontDatabase::systemFont(QFontDatabase::FixedFont).families().constFirst(), 12);

	cnt->document()->setDefaultFont(font);

	frm->setContentsMargins(0, 0, 0, 0);
	frm->addWidget(cnt, 0, 0);

	if (DialogDockWidget* dwid = qobject_cast<DialogDockWidget*>(parent))
		dwid->setLayout(frm);
	else
		frm->setParent(parent);
}

//TODO
void e2db_console_gui::init()
{
	debug("init");

	std::cout.width();

	QByteArray* ba_out = new QByteArray;
	QByteArray* ba_err = new QByteArray;

	QTextStream* ts_out = new QTextStream(ba_out);
	QTextStream* ts_err = new QTextStream(ba_err);

	this->pout = new stream(*ts_out);
	this->perr = new stream(*ts_err);

	console_header();

	ts_err->seek(0);
	ts_out->seek(0);
	if (! ts_err->readAll().isEmpty())
	{
		ts_err->seek(0);
		QTextCursor cursor = QTextCursor(cnt->document()->lastBlock());
		QTextCharFormat tf;
		tf.setForeground(QBrush(Qt::red));
		cursor.movePosition(QTextCursor::EndOfBlock);
		cursor.insertBlock();
		cursor.setCharFormat(tf);
		cursor.insertText(ts_err->readAll());
		cursor.insertBlock();
		cursor.setCharFormat(QTextCharFormat());
		cnt->setTextCursor(cursor);
	}
	{
		QTextCursor cursor = QTextCursor(cnt->document()->lastBlock());
		cursor.movePosition(QTextCursor::EndOfBlock);
		cursor.insertBlock();
		cursor.setCharFormat(QTextCharFormat());
		cursor.insertText(ts_out->readAll());
		cnt->setTextCursor(cursor);
	}
	ba_err->clear();
	ba_out->clear();
	ts_err->seek(0);
	ts_out->seek(0);

	QTextCursor cursor = QTextCursor(cnt->document()->lastBlock());
	cursor.movePosition(QTextCursor::EndOfBlock);
	cursor.insertText(">");
	cnt->setTextCursor(cursor);

	cnt->connect(cnt, &ConsoleWidget::command, [=](const QString cmd) {
		qDebug() << "cmd: " << cmd;

		std::istream* is = new std::istream(nullptr);

		if (cmd == "quit" || cmd == "exit" || cmd == "q")
			return;
		else if (cmd == "help" || cmd == "h")
			command_help(is);
		else if (cmd == "version" || cmd == "v")
			command_version();
		else if (cmd == "read" || cmd == "i")
			command_read(is);
		else if (cmd == "write" || cmd == "o")
			command_write(is);
		else if (cmd == "list" || cmd == "l")
			command_list(is);
		else if (cmd == "add" || cmd == "a")
			command_add(is);
		else if (cmd == "edit" || cmd == "e")
			command_edit(is);
		else if (cmd == "remove" || cmd == "r")
			command_remove(is);
		// else if (cmd == "copy" || cmd == "c")
		// 	command_copy(is);
		// else if (cmd == "move" || cmd == "m")
		// 	command_move(is);
		else if (cmd == "set" || cmd == "s")
			command_set(is);
		else if (cmd == "unset" || cmd == "u")
			command_unset(is);
		else if (cmd == "print" || cmd == "p")
			command_print(is);
		else if (cmd == "import")
			command_import(is);
		else if (cmd == "export")
			command_export(is);
		else if (cmd == "merge")
			command_merge(is);
		else if (cmd == "parse")
			command_parse(is);
		else if (cmd == "make")
			command_make(is);
		else if (cmd == "convert")
			command_convert(is);
		else if (cmd == "tool")
			command_tool(is);
		else if (cmd == "macro")
			command_macro(is);
		else if (cmd == "inspect")
			command_inspect(is);
		// else if (cmd == "preferences")
		// 	command_preferences(is);
		else if (! cmd.isEmpty())
			console_error(cmd.toStdString());

		ts_err->seek(0);
		ts_out->seek(0);
		if (! ts_err->readAll().isEmpty())
		{
			ts_err->seek(0);
			QTextCursor cursor = QTextCursor(cnt->document()->lastBlock());
			QTextCharFormat tf;
			tf.setForeground(QBrush(Qt::red));
			cursor.movePosition(QTextCursor::EndOfBlock);
			cursor.insertBlock();
			cursor.insertText(ts_err->readAll());
			cursor.insertBlock();
			cursor.setCharFormat(tf);
			cnt->setTextCursor(cursor);
		}
		{
			QTextCursor cursor = QTextCursor(cnt->document()->lastBlock());
			cursor.movePosition(QTextCursor::EndOfBlock);
			cursor.insertBlock();
			cursor.setCharFormat(QTextCharFormat());
			cursor.insertText(ts_out->readAll());
			cnt->setTextCursor(cursor);
		}
		ba_err->clear();
		ba_out->clear();
		ts_err->seek(0);
		ts_out->seek(0);

		QTextCursor cursor = QTextCursor (cnt->document()->lastBlock());
		cursor.movePosition(QTextCursor::EndOfBlock);
		cursor.insertText(">");
		cnt->setTextCursor(cursor);

		delete is;
	});
}

e2db_console_gui::~e2db_console_gui()
{
}

}
