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

#include <QApplication>
#include <QMimeData>

#include "../platforms/platform.h"

#include "ConsoleWidget.h"

namespace e2se_gui
{

ConsoleWidget::ConsoleWidget(QWidget* parent) : QPlainTextEdit(parent)
{
	this->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this, &ConsoleWidget::customContextMenuRequested, this, [=](QPoint pos) {
		this->showContextMenu(pos);
	}); // moc
}

void ConsoleWidget::attach(QWidget* parent)
{
	this->setParent(parent);
	this->setUpdatesEnabled(true);
	this->blockSignals(false);

	this->ruler();
}

void ConsoleWidget::detach()
{
	this->blockSignals(true);
	this->setUpdatesEnabled(false);
	this->setParent(nullptr);
}

void ConsoleWidget::output(const QString text)
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	this->maybeInsertBlock(cursor);
	cursor.setCharFormat(QTextCharFormat());
	cursor.insertText(text);
	this->setTextCursor(cursor);

	this->gtpos = cursor.position();
}

void ConsoleWidget::error(const QString text)
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	QTextCharFormat tf;
	tf.setForeground(QBrush(Qt::red));
	this->maybeInsertBlock(cursor);
	cursor.setCharFormat(tf);
	cursor.insertText(text);
	cursor.setCharFormat(QTextCharFormat());
	this->setTextCursor(cursor);

	this->gtpos = cursor.position();
}

void ConsoleWidget::prompt()
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	this->maybeInsertBlock(cursor);	
	cursor.setCharFormat(QTextCharFormat());
	cursor.insertText(">");
	this->setTextCursor(cursor);

	this->gtpos = cursor.position();
	this->currhr = HANDLE::Command;
}

void ConsoleWidget::nav()
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	this->maybeInsertBlock(cursor);
	cursor.insertText("Press key [Up] | [Down] to Move, [q] to Exit");
	this->setTextCursor(cursor);

	this->gtpos = cursor.position();
	this->currhr = HANDLE::Listing;
}

void ConsoleWidget::beep()
{
	QApplication::beep();
}

void ConsoleWidget::ruler()
{
	QTextCursor cursor = QTextCursor(this->document()->end());
	cursor.insertBlock();
	cursor.insertText("\n");
}

void ConsoleWidget::maybeInsertBlock(QTextCursor &cursor)
{
	if (! this->document()->lastBlock().text().isEmpty())
	{
	  cursor.movePosition(QTextCursor::EndOfBlock);
	  cursor.insertBlock();
	}
}

void ConsoleWidget::keyPressEvent(QKeyEvent* event)
{
	QTextCursor cursor = this->textCursor();

	if (event->matches(QKeySequence::Copy))
	{
		return QPlainTextEdit::keyPressEvent(event);
	}
	else if (cursor.selectionStart() < this->gtpos)
	{
		cursor.movePosition(QTextCursor::End);
		this->setTextCursor(cursor);

		return;
	}

	if (this->currhr == HANDLE::Listing)
	{
		emit input(static_cast<Qt::Key>(event->key()), NULL);
		return;
	}
	else
	{
		if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
		{
			QTextCursor cursor = QTextCursor(this->document()->lastBlock());
			cursor.setPosition(this->gtpos, QTextCursor::MoveAnchor);
			cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

			emit input(Qt::Key_Return, cursor.selectedText());
			return;
		}
		else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
		{
			if (cursor.position() <= this->gtpos)
				return this->beep();
		}
		else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
		{
			if (cursor.position() <= this->gtpos)
				return this->beep();
			else if (this->currhr == HANDLE::Command)
				emit input(static_cast<Qt::Key>(event->key()), NULL);

			return;
		}
		else if (cursor.position() <= this->gtpos)
		{
			if (
				event->key() == Qt::Key_Backspace ||
				event->key() == Qt::Key_Delete ||
				event->matches(QKeySequence::Paste)
			)
				return this->beep();
		}
	}

	QPlainTextEdit::keyPressEvent(event);
}

bool ConsoleWidget::canInsertFromMimeData(const QMimeData* source) const
{
	if (this->textCursor().position() <= this->gtpos)
		return false;

	return QPlainTextEdit::canInsertFromMimeData(source);
}

void ConsoleWidget::insertFromMimeData(const QMimeData* source)
{
	QPlainTextEdit::insertFromMimeData(source);

	emit input(0, source->text());
}

void ConsoleWidget::showContextMenu(QPoint pos)
{
	QMenu* menu = new QMenu;

	{
		QAction* action = new QAction(menu);
		action->setText(tr("&Copy", "context-menu"));
		action->setEnabled(this->textCursor().hasSelection());
		connect(action, &QAction::triggered, this, &ConsoleWidget::copy); // moc
		menu->addAction(action);
	}
	{
		QAction* action = new QAction(menu);
		action->setText(tr("&Paste", "context-menu"));
		action->setEnabled((this->textCursor().position() <= this->gtpos));
		connect(action, &QAction::triggered, this, &ConsoleWidget::paste); // moc
		menu->addAction(action);
	}

	platform::osMenuPopup(menu, this, pos);
}

}
