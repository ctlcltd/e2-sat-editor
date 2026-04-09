/*!
 * e2-sat-editor/src/gui/toolkit/ConsoleWidget.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "ConsoleWidget.h"

namespace e2se_gui
{

ConsoleWidget::ConsoleWidget(QWidget* parent) : QPlainTextEdit(parent)
{
}

void ConsoleWidget::prompt()
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());

	if (! this->document()->lastBlock().text().isEmpty())
	{
	  cursor.movePosition(QTextCursor::EndOfBlock);
	  cursor.insertBlock();
	}
	cursor.setCharFormat(QTextCharFormat());
	cursor.insertText(">");
	this->setTextCursor(cursor);

	this->gtpos = this->textCursor().position();
}

void ConsoleWidget::prompt(const QString cmd)
{
	this->prompt();

	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	cursor.insertText(cmd);
	this->setTextCursor(cursor);
}

void ConsoleWidget::nav(const QString text)
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());

	if (! this->document()->lastBlock().text().isEmpty())
	{
	  cursor.movePosition(QTextCursor::EndOfBlock);
	  cursor.insertBlock();
	}
	cursor.insertText(text);
	this->setTextCursor(cursor);
}

void ConsoleWidget::keyPressEvent(QKeyEvent* event)
{
	QTextCursor cursor = this->textCursor();

	if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C)
	{
		return QPlainTextEdit::keyPressEvent(event);
	}
	else if (cursor.selectionStart() < this->gtpos)
	{
		cursor.movePosition(QTextCursor::End);
		this->setTextCursor(cursor);

		return;
	}

	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		emit input(Qt::Key_Return, document()->lastBlock().text());
		return;
	}
	else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
	{
		if (cursor.position() <= this->gtpos)
			return;
	}
	else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
	{
		if (cursor.position() <= this->gtpos)
			return;

		emit input(static_cast<Qt::Key>(event->key()), NULL);
		return;
	}
	else if (event->key() == Qt::Key_Backspace && cursor.position() <= this->gtpos)
	{
		return;
	}

	QPlainTextEdit::keyPressEvent(event);
}

}
