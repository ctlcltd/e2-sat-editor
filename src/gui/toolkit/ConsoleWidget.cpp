/*!
 * e2-sat-editor/src/gui/toolkit/ConsoleWidget.cpp
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 2.0.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <QApplication>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QTextDocument>

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

	this->setCursorWidth(7);
	this->setLineWrapMode(QPlainTextEdit::NoWrap);
	this->setWordWrapMode(QTextOption::NoWrap);
	this->setAcceptDrops(false);
	this->setUndoRedoEnabled(false);
	this->setBackgroundVisible(false);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	
	QFont font = QFont ();
	// reflects qtbase/src/gui/text/qtexthtmlparser.cpp for <pre>
	font.setFamily(QFontDatabase::systemFont(QFontDatabase::FixedFont).families().constFirst());
	font.setPixelSize(12);
	this->document()->setDefaultFont(font);

	// this->setTabStopDistance();
}

void ConsoleWidget::attachWidget()
{
	this->setUpdatesEnabled(true);
	this->blockSignals(false);
}

void ConsoleWidget::detachWidget()
{
	this->blockSignals(true);
	this->setUpdatesEnabled(false);
	this->setParent(nullptr);
}

void ConsoleWidget::printOutput(const QString text)
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());

	if (this->imval)
		cursor.setPosition(this->impos);
	else
		this->maybeInsertBlock(cursor);

	cursor.setCharFormat(QTextCharFormat());
	cursor.insertText(text);
	this->setTextCursor(cursor);
	this->ensureCursorVisible();
}

void ConsoleWidget::printErrors(const QString text)
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	QTextCharFormat tf;
	tf.setForeground(QBrush(Qt::red));
	this->maybeInsertBlock(cursor);
	cursor.setCharFormat(tf);
	cursor.insertText(text);
	cursor.setCharFormat(QTextCharFormat());
	this->setTextCursor(cursor);
	this->ensureCursorVisible();
}

void ConsoleWidget::printHistory(const Qt::Key key, const QString text)
{
	if (this->currhr != HANDLE::Command || (key != Qt::Key_Up && key != Qt::Key_Down))
		return;

	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	cursor.setPosition(this->tcpos, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	cursor.removeSelectedText();
	cursor.setCharFormat(QTextCharFormat());
	cursor.insertText(text);
	this->setTextCursor(cursor);
	this->ensureCursorVisible();
}

void ConsoleWidget::printPromptCursor()
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	this->maybeInsertBlock(cursor);
	cursor.setCharFormat(QTextCharFormat());
	cursor.insertText("> ");
	this->setTextCursor(cursor);
	this->ensureCursorVisible();

	this->tcpos = cursor.position();
}

void ConsoleWidget::printNavigationRuler()
{
	if (this->nblen != 0)
	{
		QTextCursor cursor = QTextCursor(this->document());
		cursor.setPosition(this->nbpos);
		QTextBlock block = cursor.block();

		if (block.length() == this->nblen)
		{
			cursor.setPosition(block.position(), QTextCursor::MoveAnchor);
			cursor.setPosition((block.position() + block.length()), QTextCursor::KeepAnchor);
			cursor.removeSelectedText();
		}
	}

	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	this->nbpos = cursor.position();
	this->maybeInsertBlock(cursor);
	cursor.setCharFormat(QTextCharFormat());
	cursor.insertText("Press key [Up] | [Down] to Move, [q] to Exit");
	this->setTextCursor(cursor);
	this->ensureCursorVisible();

	this->nblen = cursor.block().length();
}

void ConsoleWidget::printSessionRuler()
{
	QTextCursor cursor = QTextCursor(this->document()->lastBlock());
	cursor.movePosition(QTextCursor::EndOfBlock);
	cursor.insertBlock();
	cursor.insertText("\n");
}

ConsoleWidget::HANDLE ConsoleWidget::currentHandler() const
{
	return this->currhr;
}

void ConsoleWidget::setCurrentHandler(HANDLE handle)
{
	this->currhr = handle;
	this->nbpos = 0;
	this->nblen = 0;
}

void ConsoleWidget::setCurrentHandler(int handle)
{
	this->currhr = static_cast<HANDLE>(handle);
}

bool ConsoleWidget::isInputMasked() const
{
	return this->imval;
}

void ConsoleWidget::setInputMasked(bool masked)
{
	this->imval = masked;
	this->impos = this->textCursor().position();
}

void ConsoleWidget::reset()
{
	this->currhr = HANDLE::Command;
	this->tcpos = 0;
	this->impos = 0;
	this->imval = false;
	this->nbpos = 0;
	this->nblen = 0;
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
	else if (cursor.selectionStart() < this->tcpos)
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
			cursor.setPosition(this->tcpos, QTextCursor::MoveAnchor);
			cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

			emit input(Qt::Key_Return, cursor.selectedText());
			return;
		}
		else if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
		{
			if (cursor.position() <= this->tcpos)
				return QApplication::beep();
		}
		else if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
		{
			if (cursor.position() < this->tcpos)
			{
				return QApplication::beep();
			}
			else if (this->currhr == HANDLE::Command)
			{
				QTextCursor cursor = QTextCursor(this->document()->lastBlock());
				cursor.setPosition(this->tcpos, QTextCursor::MoveAnchor);
				cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

				emit input(static_cast<Qt::Key>(event->key()), cursor.selectedText());
				return;
			}

			return;
		}
		else if (cursor.position() <= this->tcpos)
		{
			if (
				event->key() == Qt::Key_Backspace ||
				event->key() == Qt::Key_Delete ||
				event->matches(QKeySequence::Paste)
			)
				return QApplication::beep();
		}
	}

	QPlainTextEdit::keyPressEvent(event);
}

bool ConsoleWidget::canInsertFromMimeData(const QMimeData* source) const
{
	if (this->textCursor().position() <= this->tcpos)
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
		//TODO context [this] or menu
		connect(action, &QAction::triggered, this, &ConsoleWidget::copy); // moc
		menu->addAction(action);
	}
	{
		QAction* action = new QAction(menu);
		action->setText(tr("&Paste", "context-menu"));
		action->setEnabled((this->textCursor().position() <= this->tcpos));
		//TODO context [this] or menu
		connect(action, &QAction::triggered, this, &ConsoleWidget::paste); // moc
		menu->addAction(action);
	}

	platform::osMenuPopup(menu, this, pos);
}

}
